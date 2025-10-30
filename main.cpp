#include "includes/RunTime.hpp"

void signalHandler(int signum) {
    if (signum == SIGINT) {
        std::cout << "\n[Signal] SIGINT received, shutting down gracefully..." << std::endl;
        RunTime::deleteInstance();
    }
    else if (signum == SIGPIPE) {
        //| Ignorar SIGPIPE - evita que o servidor crash quando cliente desconecta durante write
        std::cerr << "[Signal] SIGPIPE received and ignored (client disconnected during write)" << std::endl;
    }
}

int	verifyArgs(int ac, char **av)
{
	if (ac > 2)
	{
		std::cerr << "Usage: " << av[0] << " <config_file>\n\tOR\nUsage: " << av[0] << std::endl;
		return (0);
	}
	return (1);
}

void	printBlock(std::vector<ServerBlock> serverBlocks, std::vector<ServerListen> serverListens)
{
    for (size_t i = 0; i < serverBlocks.size(); i++)
    {
        std::cout << "==================== SERVER BLOCK " << i + 1 << " ====================" << std::endl;
        serverBlocks[i].printServerBlock();
        std::cout << "Listens: " << std::endl;
        for (size_t j = 0; j < serverListens.size(); j++)
        {
            if (serverListens[j].getServerBlock() == serverBlocks[i])
            {
                std::cout << "Host[" << j << "]: " << serverListens[j].getHost() << " Port[" << j << "]: " << serverListens[j].getPort() << std::endl;
            }
        }
        std::map<std::string, LocationBlock> locations = serverBlocks[i].getLocations();
        for (std::map<std::string, LocationBlock>::iterator it = locations.begin(); it != locations.end(); it++)
        {
            std::cout << "------------------ LOCATION BLOCK ------------------" << std::endl;
            it->second.printLocationBlock();
            std::cout << "----------------------------------------------------" << std::endl;
        }
        std::cout << "========================================================" << std::endl;
    }
}

void epollReadyListLoop(int numberOfReadySockets) {
    if (numberOfReadySockets) {
        for (int i = 0; i < numberOfReadySockets; i++) {
            struct epoll_event &data = RunTime::getEpoll().getElementFromReadyList(i);
            
            EpollHandler *handler = static_cast<EpollHandler *>(data.data.ptr);
            if (handler) {
                handler->handleEvent(data);
            }
        }
    }
    
    //| Verificar CGIs em execução e clientes inativos
    //| Coletar FDs a serem deletados
    std::vector<int> fdsToDelete;
    std::map<int, Client> &clients = RunTime::getClients();
    
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        Client &client = it->second;
        int clientFd = client.getSocketFd();
        
        //| 1. Verificar timeout de clientes inativos (30 segundos)
        if (client.isTimedOut(30)) {
            std::cout << "[Timeout] Client " << clientFd << " inactive for >30s, closing connection" << std::endl;
            fdsToDelete.push_back(clientFd);
            continue;
        }
        
        //| 2. Se client está executando CGI, verificar se completou
        if (client.getState() == EXECUTING_CGI) {
            client.checkCgiCompletion();
            
            //| Se CGI completou, enviar resposta
            if (client.getState() == COMPLETE) {
                std::string responseStr = client.getResponse().toString();
                std::cout << "============ CGI RESPONSE SEND ============" << std::endl;
                std::cout << responseStr << std::endl;
                std::cout << "===========================================" << std::endl;
                send(clientFd, responseStr.c_str(), responseStr.size(), 0);
                fdsToDelete.push_back(clientFd);
            }
        }
    }
    
    //| Deletar clientes coletados
    for (size_t i = 0; i < fdsToDelete.size(); ++i) {
        RunTime::deleteClient(fdsToDelete[i]);
    }
}

void serverMainLoop() {
    while (true) {
        int numberOfReadySockets = RunTime::getEpoll().manipEpollWait();
        if (numberOfReadySockets == -1) {
            std::cerr << "Error: erro ao manipular o epoll_wait()." << std::endl;
            break;
        }
        else {
            epollReadyListLoop(numberOfReadySockets);
        }
    }
}

int main(int ac, char **av) {
    if (!verifyArgs(ac, av))
        return (1);

    //| Registrar signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGPIPE, signalHandler);  //| Evitar crash quando cliente desconecta durante write

    try {
        RunTime::initializeRuntime(ac, av);
        printBlock(RunTime::getConfig().getServerBlocks(), RunTime::getServerListeners());
        serverMainLoop();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        RunTime::deleteInstance();
        return (-1);
    }
    RunTime::deleteInstance();

    return (0);
}

#include "includes/RunTime.hpp"

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

void clientSocketIsReady(RunTime *runtime, struct epoll_event &clientSocket) {
    int clientFd = clientSocket.data.fd;
    
    std::map<int, Client>::iterator it = runtime->_clients.find(clientFd);
    if (it == runtime->_clients.end()) {
        std::cerr << "Client not found in the map." << std::endl;
        return;
    }
    Client &client = it->second; 
    char buffer[5] = {0};
    int count = 0;
    // (void)runtime;
    
    // Precisamos de uma forma de identificar o final da request.
    // Dessa forma, conseguimos setar o state do client para COMPLETE
    if (clientSocket.events & EPOLLIN) {
        if ((count = read(clientFd, buffer, 1)) > 0) {
            // aqui, leremos o que o cliente esta mandando para o servidor.
            // Faremos uma leitura em chuncks! Isto e, leremos de pouco em pouco.
            // provavelmente nunca leremos todo o conteudo da requisicao de uma vez so.
            client.concatenateRequestData(buffer);
            if (client.isRequestComplete()) {
                // Ao chegar aqui, ja lemos toda a request do cliente.
                // Nessa etapa, precisamos parsear a request
                // Processar o que for necessario
                // Montar a response
                // Enviar a response ao cliente
                // close() no fd do client
                // .erase() do map
                std::cout << "================== REQUEST COMPLETE =================" << std::endl;
                std::cout << client.request.getMethod() << std::endl;
                std::cout << client.request.getUri() << std::endl;
                std::map<std::string, std::string> headers = client.request.getHeaders();
                for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++) {
                    std::cout << it->first << ": " << it->second << std::endl;
                }
                std::cout << "Body: " << client.request.getBody() << std::endl;
                std::cout << "=====================================================" << std::endl;
                client.response = HttpResponse(client.request);
                std::string responseStr = client.response.toString();
                std::cout << "=================== RESPONSE SEND ===================" << std::endl;
                std::cout << responseStr << std::endl;
                std::cout << "=====================================================" << std::endl;
                send(clientFd, responseStr.c_str(), responseStr.size(), 0);
                runtime->deleteClient(clientFd);
            }
            // no momento, so estamos printando na tela mesmo.
            //Precisamos de alguma forma de armazenar o conteudo ja lido de algum socket
            // em alguma estrutura para que, durante as proximas leituras, possamos concatenar
            // o que ja lemos com o que acabamos de ler.
        } else if (count == 0) {
            std::cout << "Client closed the connection." << std::endl;
            std::cout << client.getRawRequest() << std::endl;
            runtime->deleteClient(clientFd);
        }
    }
    if (clientSocket.events & EPOLLRDHUP) {
        std::cout << "Erro capturado pelo epoll." << std::endl;
        std::cout << client.getRawRequest() << std::endl;
        runtime->deleteClient(clientFd);
    }
}

void serverSocketIsReady(RunTime *runtime, struct epoll_event &serverSocket) {
    std::cout << "FD do server: " << serverSocket.data.fd << std::endl;
    while (true) {
        struct sockaddr_in clientSocketAddr;
        socklen_t clientSocketLength = sizeof(clientSocketAddr);
        int clientFd = accept(serverSocket.data.fd, (struct sockaddr *)&clientSocketAddr, &clientSocketLength);

        std::cout << "comecou" << std::endl;
        if (clientFd == -1) {
            std::cout << "Erro accept" << std::endl;
            std::cout << clientFd << std::endl;
            std::cout << "errno: " << errno << std::endl;
            break;
            //EAGAIN or EWOULDBLOCK
            //The socket is marked nonblocking and no connections are
            //present to be accepted (nao ha mais conexoes para serem aceitas)
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cout << "BREAK;" << std::endl;
                break;
            }
        } else {
            try {
                // Adicionar o novo socket do cliente no map de clientes.
                // A ideia aqui e que, sempre quando um novo cliente realizar uma conexao com o servidor
                // Precisaremos ler o conteudo da request, armazenar, parsear, processar e devolver
                // Com o map, esses processos devem ficar mais tranquilos e simples/rapidos
                set_nonblocking(clientFd);
                // runtime->_clients.insert(
                //     std::make_pair(clientFd, Client(clientFd, runtime->_config.getServerListens()[0]))
                // );
                // std::cout << "Nao inseriu no map" << std::endl;
                runtime->_clients.insert(
                    std::make_pair(clientFd, Client(clientFd, runtime->_config.getElementInServerList(serverSocket.data.fd)))
                );
                std::cout << "inseriu novo client no map." << std::endl;
                runtime->_epoll.manipInterestList(EPOLL_CTL_ADD, EPOLLIN | EPOLLRDHUP, clientFd, 0);
            }
            catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
                close(clientFd);
            }
        }
    }
    std::cout << "acabou" << std::endl;
    
}

void epollReadyListLoop(RunTime *runtime, int numberOfReadySockets) {
    if (numberOfReadySockets) {
        for (int i = 0; i < numberOfReadySockets; i++) {
            struct epoll_event &socketReady = runtime->_epoll.getElementFromReadyList(i);
            // struct epoll_event &socketReady = runtime->_epoll.getReadyList();
    
            if (socketReady.data.ptr != NULL) {
                std::cout << "Evento ocorreu no serverSocket." << std::endl;
                std::cout << "O FD e: " << socketReady.data.fd << std::endl;
                serverSocketIsReady(runtime, socketReady);
                return ;
            }
            else {
                std::cout << "Evento ocorreu com um clientSocket." << std::endl;
                //Validar se ja existe alguma key no map de clientes com o valor do FD do clientFd.
                //Caso ja exista, nao faz nada????
                //Caso nao exista, adiciona mais um elemento no map
                clientSocketIsReady(runtime, socketReady);
            }
            break;
        }
    }
}

void serverMainLoop(RunTime *runtime) {
    while (true) {
        int numberOfReadySockets = runtime->_epoll.manipEpollWait();
        if (numberOfReadySockets == -1) {
            std::cerr << "Error: erro ao manipular o epoll_wait()." << std::endl;
            break;
        }
        else if (numberOfReadySockets >= 1){
            epollReadyListLoop(runtime, numberOfReadySockets);
            break;
        }
    }
}

void parseConfigFile(RunTime *runtime, int ac, char **av)
{
    if (ac == 2)
        runtime->_config.parser(av[1]);
    else //| Caso não passem nenhum argumento, vamos usar nosso arquivo padrão
        runtime->_config.parser("configs/test_simple.conf");
    printBlock(runtime->_config.getServerBlocks(), runtime->_config.getServerListens());
}

int main(int ac, char **av) {
    if (!verifyArgs(ac, av))
        return (1);

    // RunTime runtime(AF_INET, SOCK_STREAM);
    RunTime runtime(av, ac);

    try {
        printBlock(runtime._config.getServerBlocks(), runtime._config.getServerListens());

        // Criar um novo construtor para a Classe RunTime.
        // Esse novo construtor vai fazer ser responsavel por parsear o .conf
        //  Caso tenha dado certo o parser, ele continua. Se nao, ja joga um exception.
        //  Em sucesso, ele cria e abastece todos os atributos necessarios para o servidor rodar.
        //      Isto e: 
        //          os sockets do servidor em modo passivo
        //          setados para nao bloqueante
        //          cria a instancia de epoll
        //          seleciona o modo correto do epoll
        //          abastece a insterest list com os fds dos sockets do servidor
        //          
        
        // parseConfigFile(&runtime, ac, av);
        // runtime._server.setServerAddr(
        //     AF_INET,
        //     runtime._config.getServerListens()[0].getPort(),
        //     runtime._config.getServerListens()[0].getHost()
        // );
        // runtime._server.bindServerSocket();
        // runtime._server.updateToNonBlocking();
        // runtime._server.listenServerSocket();
        // runtime._epoll.manipInterestList(EPOLL_CTL_ADD, EPOLLIN, runtime._server.getServerFd());
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return (-1);
    }

    serverMainLoop(&runtime);

    return (0);
}

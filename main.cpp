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

void clientSocketIsReady(RunTime *runtime, struct epoll_event &clientSocket, int clientFd) {
    std::map<int, Client>::iterator it = runtime->clients.find(clientFd);
    if (it == runtime->clients.end()) {
        std::cerr << "Client not found in the map." << std::endl;
        return;
    }
    Client &client = it->second; 
    char buffer[5] = {0};
    int count = 0;

    if (clientSocket.events & EPOLLIN) {
        if ((count = read(clientFd, buffer, 1)) > 0) {
            client.concatenateRequestData(buffer);
            if (client.isRequestComplete()) {
                std::cout << "================== REQUEST COMPLETE =================" << std::endl;
                std::cout << client.request.getMethod() << std::endl;
                std::cout << client.request.getUri() << std::endl;
                std::map<std::string, std::string> headers = client.request.getHeaders();
                for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++) {
                    std::cout << it->first << ": " << it->second << std::endl;
                }
                std::cout << "Body: " << client.request.getBody() << std::endl;
                std::cout << "=====================================================" << std::endl;
                client.response.dispatchRequest(client.request);
                std::string responseStr = client.response.toString();
                std::cout << "=================== RESPONSE SEND ===================" << std::endl;
                std::cout << responseStr << std::endl;
                std::cout << "=====================================================" << std::endl;
                send(clientFd, responseStr.c_str(), responseStr.size(), 0);
                runtime->deleteClient(clientFd);
            }
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

void serverSocketIsReady(RunTime *runtime, int serverFd) {
    while (true) {
        struct sockaddr_in clientSocketAddr;
        socklen_t clientSocketLength = sizeof(clientSocketAddr);
        int clientFd = accept(serverFd, (struct sockaddr *)&clientSocketAddr, &clientSocketLength);

        if (clientFd == -1) {
            //EAGAIN or EWOULDBLOCK
            //The socket is marked nonblocking and no connections are
            //present to be accepted (nao ha mais conexoes para serem aceitas)
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // std::cout << "BREAK;" << std::endl;
                break;
            }
        } else {
            try {
                set_nonblocking(clientFd);
                runtime->clients.insert(
                    std::make_pair(clientFd, Client(clientFd, runtime->getElementInServerList(serverFd)))
                );
                std::cout << "inseriu novo client no map." << std::endl;
                runtime->epoll.manipInterestList(EPOLL_CTL_ADD, EPOLLIN | EPOLLRDHUP, clientFd, 0);
            }
            catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
                close(clientFd);
            }
        }
        return ;
    }
}

void epollReadyListLoop(RunTime *runtime, int numberOfReadySockets) {
    if (numberOfReadySockets) {
        for (int i = 0; i < numberOfReadySockets; i++) {
            struct epoll_event &socketReady = runtime->epoll.getElementFromReadyList(i);
            struct epollUserData *data = (struct epollUserData *)socketReady.data.ptr;
    
            if (data->isServerSocket) {
                std::cout << "Evento ocorreu no serverSocket." << std::endl;
                std::cout << "O FD e: " << data->fd << std::endl;
                serverSocketIsReady(runtime, data->fd);
                return;
            }
            else {
                std::cout << "Evento ocorreu com um clientSocket." << std::endl;
                std::cout << "O FD e: " << data->fd << std::endl;
                clientSocketIsReady(runtime, socketReady, data->fd);
            }
        }
    }
}

void serverMainLoop(RunTime *runtime) {
    while (true) {
        int numberOfReadySockets = runtime->epoll.manipEpollWait();
        if (numberOfReadySockets == -1) {
            std::cerr << "Error: erro ao manipular o epoll_wait()." << std::endl;
            break;
        }
        else {
            epollReadyListLoop(runtime, numberOfReadySockets);
        }
    }
}

int main(int ac, char **av) {
    if (!verifyArgs(ac, av))
        return (1);

    try {
        RunTime runtime(av, ac);
        printBlock(runtime.config.getServerBlocks(), runtime.getServerListeners());
        serverMainLoop(&runtime);
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return (-1);
    }

    return (0);
}

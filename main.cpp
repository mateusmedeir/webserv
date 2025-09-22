#include "WebservHeader.hpp"

void clientSocketIsReady(RunTime *runtime, struct epoll_event &clientSocket) {
    int clientFd = clientSocket.data.fd;
    char buffer[1024];
    int count = 0;
    (void)runtime;

    if (clientSocket.events & EPOLLIN) {
        while((count = read(clientFd, buffer, 1024)) > 0) {
            // aqui, leremos o que o cliente esta mandando para o servidor.
            // Faremos uma leitura em chuncks! Isto e, leremos de pouco em pouco.
            // provavelmente nunca leremos todo o conteudo da requisicao de uma vez so.
            write(STDOUT_FILENO, buffer, count);
            // no momento, so estamos printando na tela mesmo.
            //Precisamos de alguma forma de armazenar o conteudo ja lido de algum socket
            // em alguma estrutura para que, durante as proximas leituras, possamos concatenar
            // o que ja lemos com o que acabamos de ler.
        }
    }
    if (clientSocket.events & EPOLLRDHUP) {
        std::cout << "Erro capturado pelo epoll." << std::endl;
        close(clientFd);
    }
}

void serverSocketIsReady(RunTime *runtime) {
    while (true) {
        struct sockaddr_in clientSocketAddr;
        socklen_t clientSocketLength = sizeof(clientSocketAddr);
        int clientFd = accept(runtime->_server.getServerFd(), (struct sockaddr *)&clientSocketAddr, &clientSocketLength);

        if (clientFd == -1) {
            //EAGAIN or EWOULDBLOCK
            //The socket is marked nonblocking and no connections are
            //present to be accepted (nao ha mais conexoes para serem aceitas)
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
        } else {
            set_nonblocking(clientFd);
            try {
                runtime->_epoll.manipInterestList(EPOLL_CTL_ADD, EPOLLIN | EPOLLRDHUP, clientFd);
            }
            catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
                close(clientFd);
            }
        }
    }
    
}

void epollReadyListLoop(RunTime *runtime, int numberOfReadySockets) {
    for (int i = 0; i < numberOfReadySockets; i++) {
        struct epoll_event &socketReady = runtime->_epoll.getElementFromReadyList(i);

        if (socketReady.data.fd == runtime->_server.getServerFd()) {
            std::cout << "Evento ocorreu no serverSocket." << std::endl;
            serverSocketIsReady(runtime);
        }
        else {
            std::cout << "Evento ocorreu com um clientSocket." << std::endl;
            clientSocketIsReady(runtime, socketReady);
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
        else if (numberOfReadySockets > 0) {
            epollReadyListLoop(runtime, numberOfReadySockets);
        }
    }
}

int main(void) {
    RunTime runtime(AF_INET, SOCK_STREAM);

    try {
        runtime._server.setServerAddr(AF_INET, 8080, INADDR_ANY);
        runtime._server.bindServerSocket();
        runtime._server.updateToNonBlocking();
        runtime._server.listenServerSocket();
        runtime._epoll.manipInterestList(EPOLL_CTL_ADD, EPOLLIN, runtime._server.getServerFd());
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return (-1);
    }

    serverMainLoop(&runtime);

    return (0);
}
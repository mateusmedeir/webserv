#include "../includes/WebservHeader.hpp"
#include "../includes/RunTime.hpp"

ServerListen::ServerListen(unsigned int host, int port, const ServerBlock &serverBlock)
    : EpollHandler(EPOLLIN | EPOLLRDHUP), _host(host), _port(port), _serverBlock(serverBlock) {}

ServerListen::ServerListen(const ServerListen &src)
    : EpollHandler(src.getSocketFd(), src.getInterestedEvents()), _host(src._host), _port(src._port), _serverBlock(src._serverBlock) {}

ServerListen &ServerListen::operator=(const ServerListen &src) {
    if (this != &src) {
        this->_host = src._host;
        this->_port = src._port;
        this->setSocketFd(src.getSocketFd());
    }
    return (*this);
}

bool ServerListen::operator==(const ServerListen &other) const {
    return (this->_host == other._host && this->_port == other._port && this->getSocketFd() == other.getSocketFd());
}

ServerListen::~ServerListen(void) {}

void ServerListen::handleEpollIn(void) {
    std::cout << "New connection incoming on server socket FD: " << this->getSocketFd() << std::endl;
    while (true) {
        struct sockaddr_in clientSocketAddr;
        socklen_t clientSocketLength = sizeof(clientSocketAddr);
        int clientFd = accept(this->getSocketFd(), (struct sockaddr *)&clientSocketAddr, &clientSocketLength);

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
                
                // ✅ Habilitar TCP_NODELAY para reduzir latência (desabilita algoritmo de Nagle)
                int flag = 1;
                if (setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) < 0) {
                    std::cerr << "[Warning] Failed to set TCP_NODELAY on client socket" << std::endl;
                }
                
                RunTime::getClients().insert(
                    std::make_pair(clientFd, Client(clientFd, RunTime::getElementInServerList(this->getSocketFd())))
                );
                std::cout << "inseriu novo client no map." << std::endl;
                RunTime::getEpoll().manipInterestList(EPOLL_CTL_ADD, &RunTime::getClient(clientFd));
            }
            catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
                close(clientFd);
            }
        }
    }
}

unsigned int ServerListen::getHost(void) const {
    return (this->_host);
}

int ServerListen::getPort(void) const {
    return (this->_port);
}

ServerBlock ServerListen::getServerBlock(void) const {
    return (this->_serverBlock);
}

void ServerListen::setServerAddr(int socketDomain) {
    this->_serverAddr.sin_family = socketDomain;
    this->_serverAddr.sin_port = htons(this->getPort());
    this->_serverAddr.sin_addr.s_addr = htonl(this->getHost());
}

void ServerListen::createServerSocket(int socketDomain, int socketType) {
    this->setSocketFd(socket(socketDomain, socketType, 0));

    std::cout << "socket()" << this->getSocketFd() << std::endl;
    if (this->getSocketFd() == -1) {
        throw(ServerListen::CannotInitServerSocket());
    }
    
    //| Otimização para testes de carga, permite reusar endereço/porta imediatamente
    int opt = 1;
    if (setsockopt(this->getSocketFd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cerr << "Warning: SO_REUSEADDR failed" << std::endl;
    }
    #ifdef SO_REUSEPORT
    if (setsockopt(this->getSocketFd(), SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        std::cerr << "Warning: SO_REUSEPORT failed" << std::endl;
    }
    #endif
}

void ServerListen::bindServerSocket(void) {
    std::cout << "bind()" << this->getSocketFd() << std::endl;
    if (bind(this->getSocketFd(), (struct sockaddr *)&this->_serverAddr, sizeof(this->_serverAddr)) == -1) {
        throw(ServerListen::CannotBindServerSocket());
    }
}

void ServerListen::updateToNonBlocking(void) {
    try
    {
        std::cout << "nonblocking()" << this->getSocketFd() << std::endl;
        set_nonblocking(this->getSocketFd());
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }   
}

void ServerListen::listenServerSocket(void) {
    std::cout << "listen()" << this->getSocketFd() << std::endl;
    if (listen(this->getSocketFd(), MAX_EVENTS) == -1) {
        throw(ServerListen::CannotSetServerToListen());
    }
}

void ServerListen::initServerSocket(int socketDomain, int socketType) {
    this->createServerSocket(socketDomain, socketType);
    this->setServerAddr(socketDomain);
    this->bindServerSocket();
    this->updateToNonBlocking();
    this->listenServerSocket();
}

const char * ServerListen::CannotInitServerSocket::what() const throw() {
    return ("Error: error in creating socket with socket().");
}

const char * ServerListen::CannotBindServerSocket::what() const throw() {
    return ("Error: error in binding server socket with bind().");
}

const char * ServerListen::CannotUpdateServerToNonBlocking::what() const throw() {
    return ("Error: error in trying to set the non-blocking behavior.");
}

const char * ServerListen::CannotSetServerToListen::what() const throw() {
    return ("Error: error in setting the server to listen with listen().");
}
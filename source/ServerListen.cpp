#include "../includes/WebservHeader.hpp"

ServerListen::ServerListen(unsigned int host, int port, ServerBlock &serverBlock)
    : _host(host), _port(port), _serverBlock(serverBlock) {}

ServerListen::ServerListen(const ServerListen &src)
    : _host(src._host), _port(src._port), _serverFd(src._serverFd), _serverBlock(src._serverBlock) {}

ServerListen &ServerListen::operator=(const ServerListen &src) {
    if (this != &src) {
        this->_host = src._host;
        this->_port = src._port;
        this->_serverBlock = src._serverBlock;
        this->_serverFd = src._serverFd;
    }
    return (*this);
}

bool ServerListen::operator==(const ServerListen &other) const {
    return (this->_host == other._host && this->_port == other._port && other._serverFd);
}

ServerListen::~ServerListen(void) {}

unsigned int ServerListen::getHost(void) const {
    return (this->_host);
}

int ServerListen::getPort(void) const {
    return (this->_port);
}

ServerBlock ServerListen::getServerBlock(void) const {
    return (this->_serverBlock);
}

int ServerListen::getServerFd(void) const {
    return (this->_serverFd);
}

void ServerListen::setServerAddr(int socketDomain) {
    this->_serverAddr.sin_family = socketDomain;
    this->_serverAddr.sin_port = htons(this->getPort());
    this->_serverAddr.sin_addr.s_addr = htonl(this->getHost());
}

void ServerListen::createServerSocket(int socketDomain, int socketType) {
    this->_serverFd = socket(socketDomain, socketType, 0);
    std::cout << "socket()" << this->_serverFd << std::endl;
    if (this->_serverFd == -1) {
        throw(ServerListen::CannotInitServerSocket());
    }
}

void ServerListen::bindServerSocket(void) {
    std::cout << "bind()" << this->_serverFd << std::endl;
    if (bind(this->_serverFd, (struct sockaddr *)&this->_serverAddr, sizeof(this->_serverAddr)) == -1) {
        throw(ServerListen::CannotBindServerSocket());
    }
}

void ServerListen::updateToNonBlocking(void) {
    try
    {
        std::cout << "nonblocking()" << this->_serverFd << std::endl;
        set_nonblocking(this->_serverFd);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }   
}

void ServerListen::listenServerSocket(void) {
    std::cout << "listen()" << this->_serverFd << std::endl;
    if (listen(this->_serverFd, MAX_EVENTS) == -1) {
        throw(ServerListen::CannotSetServerToListen());
    }
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
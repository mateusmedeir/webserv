#include "../includes/WebservHeader.hpp"

ServerInstance::ServerInstance(void) {}

ServerInstance::ServerInstance(int socketDomain, int socketType) {
    std::cout << "New Run time got created." << std::endl;
    try
    {
        this->initServerSocket(socketDomain, socketType);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

ServerInstance::ServerInstance(const ServerInstance &src) {
    *this = src;
}

ServerInstance &ServerInstance::operator=(const ServerInstance &src) {
    if (this != &src) {
        this->_serverFd = src._serverFd;
        this->_serverAddr = src._serverAddr;
    }
    return (*this);
}

ServerInstance::~ServerInstance(void) {}

int ServerInstance::getServerFd(void) const {
    return (this->_serverFd);
}

void ServerInstance::setServerAddr(int socketDomain, int serverPort, int serverAddr) {
    this->_serverAddr.sin_family = socketDomain;
    this->_serverAddr.sin_port = htons(serverPort);
    this->_serverAddr.sin_addr.s_addr = htons(serverAddr);
}

void ServerInstance::initServerSocket(int socketDomain, int socketType) {
    this->_serverFd = socket(socketDomain, socketType, 0);
    if (getServerFd() == -1) {
        throw(ServerInstance::CannotInitServerSocket());
    }
}

void ServerInstance::bindServerSocket(void) {
    if (bind(getServerFd(), (struct sockaddr *)&this->_serverAddr, sizeof(this->_serverAddr)) == -1) {
        throw(ServerInstance::CannotBindServerSocket());
    }
}

void ServerInstance::updateToNonBlocking(void) {
    try
    {
        set_nonblocking(getServerFd());
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }   
}

void ServerInstance::listenServerSocket(void) {
    if (listen(getServerFd(), MAX_EVENTS) == -1) {
        throw(ServerInstance::CannotSetServerToListen());
    }
}

const char * ServerInstance::CannotInitServerSocket::what() const throw() {
    return ("Error: error in creating socket with socket().");
}

const char * ServerInstance::CannotBindServerSocket::what() const throw() {
    return ("Error: error in binding server socket with bind().");
}

const char * ServerInstance::CannotUpdateServerToNonBlocking::what() const throw() {
    return ("Error: error in trying to set the non-blocking behavior.");
}

const char * ServerInstance::CannotSetServerToListen::what() const throw() {
    return ("Error: error in setting the server to listen with listen().");
}
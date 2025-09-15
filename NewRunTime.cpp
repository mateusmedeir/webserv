#include "WebservHeader.hpp"
#include "NewRunTime.hpp"

// NewRunTime(void);
NewRunTime::NewRunTime(void) {
    std::cout << "New Run time got created." << std::endl;
    // this->_epollInstance = NewEpollInstance();
}

// NewRunTime(int socketDomain, int socketType);
NewRunTime::NewRunTime(int socketDomain, int socketType) {
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

// NewRunTime(const NewRunTime &src);
NewRunTime::NewRunTime(const NewRunTime &src): NewEpollInstance(src) {
    *this = src;
}

// NewRunTime &operator=(const NewRunTime &src);
NewRunTime &NewRunTime::operator=(const NewRunTime &src) {
    if (this != &src) {
        this->_serverFd = src._serverFd;
        this->_serverAddr = src._serverAddr;
        NewEpollInstance::operator=(src);
    }
    return (*this);
}

// ~NewRunTime(void);
NewRunTime::~NewRunTime(void) {

}

// int getServerFd(void) const;
int NewRunTime::getServerFd(void) const {
    return (this->_serverFd);
}

void NewRunTime::setServerAddr(int socketDomain, int serverPort, int serverAddr) {
    this->_serverAddr.sin_family = socketDomain;
    this->_serverAddr.sin_port = htons(serverPort);
    this->_serverAddr.sin_addr.s_addr = htons(serverAddr);
}

void NewRunTime::manipInterestList(int operation, uint32_t events, int socketFd) {
    if (operation != EPOLL_CTL_ADD && operation != EPOLL_CTL_DEL && operation != EPOLL_CTL_MOD) {
        throw(NewEpollInstance::CannotManipulateEpollInstance());
    }
    this->setConfigEpollEvents(socketFd, events);
    if (epoll_ctl(this->_epollFd, operation, socketFd, &this->_configEpollEvents) == -1) {
        throw(NewEpollInstance::CannotManipulateEpollInstance());
    }
    std::cout << "Manipulacao feita com sucesso!" << std::endl;
}

// void initServerSocket(int socketDomain, int socketType);
void NewRunTime::initServerSocket(int socketDomain, int socketType) {
    this->_serverFd = socket(socketDomain, socketType, 0);
    if (getServerFd() == -1) {
        throw(NewRunTime::CannotInitServerSocket());
        //trow alguma exception??
        //erro ao criar o socket principal do servidor
        //dar close nos FDs abertos
    }
}

// void bindServerSocket();
void NewRunTime::bindServerSocket(void) {
    if (bind(getServerFd(), (struct sockaddr *)&this->_serverAddr, sizeof(this->_serverAddr)) == -1) {
        throw(NewRunTime::CannotBindServerSocket());
        //trow alguma exception?
        //erro ao bindar o socket a uma porta/endereco
        //dar close nos FDs abertos
    }
}

// void updateToNonBlocking();
void NewRunTime::updateToNonBlocking(void) {
    if (set_nonblocking(getServerFd()) == -1) {
        throw(NewRunTime::CannotUpdateServerToNonBlocking());
        //trow alguma exception?
        //erro ao por o servidor em modo nao bloqueante
        //dar close nos FDs abertos
    }
}

// void listenServerSocket();
void NewRunTime::listenServerSocket(void) {
    if (listen(getServerFd(), MAX_EVENTS) == -1) {
        throw(NewRunTime::CannotSetServerToListen());
        //trow alguma exception?
        //erro ao colocar o servidor em modo passivo
        //dar close nos FDs abertos
    }
}

const char * NewRunTime::CannotInitServerSocket::what() const throw() {
    return ("Error: error in creating socket with socket().");
}

const char * NewRunTime::CannotBindServerSocket::what() const throw() {
    return ("Error: error in binding server socket with bind().");
}

const char * NewRunTime::CannotUpdateServerToNonBlocking::what() const throw() {
    return ("Error: error in trying to set the non-blocking behavior.");
}

const char * NewRunTime::CannotSetServerToListen::what() const throw() {
    return ("Error: error in setting the server to listen with listen().");
}
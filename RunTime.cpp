#include "RunTime.hpp"

// RunTime(void);
RunTime::RunTime(void) {}

// ~RunTime(void);
RunTime::~RunTime(void) {
    close(this->_serverFd);
}

// RunTime(const RunTime &src);
RunTime::RunTime(const RunTime &src) {
    *this = src;
}
// RunTime &operator=(const RunTime &src);
RunTime &RunTime::operator=(const RunTime &src) {
    if (this != &src) {
        this->_serverFd = src._serverFd;
        this->_serverAddr = src._serverAddr;
        // this->_epollInstance = src._epollInstance;
        // this->_configEpollEvents = src._configEpollEvents;
    }
    return (*this);
}

// // RunTime(int socketDomain, int socketType, int serverPort, int serverAddr);
// RunTime::RunTime(int socketDomain, int socketType, int serverPort, int serverAddr) {
//     this->_serverFd = socket(socketDomain, socketType, 0);
//     if (this->_serverFd == -1) {
//         //error
//         //dar close nos FDs abertos
//     }
//     this->_serverAddr.sin_family = socketDomain;
//     this->_serverAddr.sin_port = htons(serverPort);
//     this->_serverAddr.sin_addr.s_addr = htons(serverAddr);
//     if (bind(this->_serverFd, (struct sockaddr *)&this->_serverAddr, sizeof(this->_serverAddr)) == -1) {
//         //error
//         //dar close nos FDs abertos
//     }
//     if (set_nonblocking(this->_serverFd) == -1) {
//         //error
//         //dar close nos FDs abertos
//     }
//     if (listen(this->_serverFd, MAX_EVENTS) == -1) {
//         //error
//         //dar close nos FDs abertos
//     }
// }

// int getServerFd(void) const;
int RunTime::getServerFd(void) const {
    return (this->_serverFd);
}

// struct sockaddr_in getServerAddr(void) const;
struct sockaddr_in RunTime::getServerAddr(void) const {
    return (this->_serverAddr);
}

// void setServerAddrStruct(int socketDomain, int serverPort, int serverAddr);
void RunTime::setServerAddrStruct(int socketDomain, int serverPort, int serverAddr) {
    this->_serverAddr.sin_family = socketDomain;
    this->_serverAddr.sin_port = htons(serverPort);
    this->_serverAddr.sin_addr.s_addr = htons(serverAddr);
}

// void initServerSocket(int socketDomain, int socketType);
void RunTime::initServerSocket(int socketDomain, int socketType) {
    this->_serverFd = socket(socketDomain, socketType, 0);
    if (getServerFd() == -1) {
        //trow alguma exception??
        //erro ao criar o socket principal do servidor
        //dar close nos FDs abertos
    }
}

// void bindServerSocket();
void RunTime::bindServerSocket(void) {
    if (bind(getServerFd(), (struct sockaddr *)&getServerAddr(), sizeof(getServerAddr())) == -1) {
        //trow alguma exception?
        //erro ao bindar o socket a uma porta/endereco
        //dar close nos FDs abertos
    }
}

// void updateToNonBlocking();
void RunTime::updateToNonBlocking(void) {
    if (set_nonblocking(getServerFd()) == -1) {
        //trow alguma exception?
        //erro ao por o servidor em modo nao bloqueante
        //dar close nos FDs abertos
    }
}

// void listenServerSocket();
void RunTime::listenServerSocket(void) {
    if (listen(getServerFd(), MAX_EVENTS) == -1) {
        //trow alguma exception?
        //erro ao colocar o servidor em modo passivo
        //dar close nos FDs abertos
    }
}

// const char * Bureaucrat::GradeTooHighException::what() const throw()
// {
//     return ("Grade is too high");
// }


// class CannotInitServerSocket : public std::exception {
//     public:
//         virtual const char *what() const throw();
// };

// class CannotBindServerSocket : public std::exception {
//     public:
//         virtual const char *what() const throw();
// };

// class CannotUpdateServerToNonBlocking : public std::exception {
//     public:
//         virtual const char *what() const throw();
// };

const char * RunTime::CannotInitServerSocket::what() const throw() {
    return ("Error: error in creating socket with socket().");
}

const char * RunTime::CannotBindServerSocket::what() const throw() {
    return ("Error: error in binding server socket with bind().")
}

const char * RunTime::CannotUpdateServerToNonBlocking::what() const throw() {
    return ("Error: error when trying to set the non-blocking behavior.");
}
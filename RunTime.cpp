#include "RunTime.hpp"

// RunTime(void);
RunTime::RunTime(void) {}

// ~RunTime(void);
RunTime::~RunTime(void) {}

// RunTime(const RunTime &src);
RunTime::RunTime(const RunTime &src) {
    *this = src;
}
// RunTime &operator=(const RunTime &src);
RunTime &RunTime::operator=(const RunTime &src) {
    if (this != &src) {
        this->_serverFd = src._serverFd;
        this->_serverAddr = src._serverAddr;
        this->_epollInstance = src._epollInstance;
        this->_configEpollEvents = src._configEpollEvents;
    }
    return (*this);
}

// RunTime(int socketDomain, int socketType, int serverPort, int serverAddr);
RunTime::RunTime(int socketDomain, int socketType, int serverPort, int serverAddr) {
    this->_serverFd = socket(socketDomain, socketType, 0);
    if (this->_serverFd == -1) {
        //error
        //dar close nos FDs abertos
    }
    this->_serverAddr.sin_family = socketDomain;
    this->_serverAddr.sin_port = htons(serverPort);
    this->_serverAddr.sin_addr.s_addr = htons(serverAddr);
    if (bind(this->_serverFd, (struct sockaddr *)&this->_serverAddr, sizeof(this->_serverAddr)) == -1) {
        //error
        //dar close nos FDs abertos
    }
    if (set_nonblocking(this->_serverFd) == -1) {
        //error
        //dar close nos FDs abertos
    }
    if (listen(this->_serverFd, MAX_EVENTS) == -1) {
        //error
        //dar close nos FDs abertos
    }
}
#include "WebservHeader.hpp"

// NewEpollInstance(void);
NewEpollInstance::NewEpollInstance() {
    //chamar metodo que via criar o epoll com o epollCreate()
    std::cout << "New instance of epoll got created." << std::endl;
    initEpollInstance();
}

// NewEpollInstance(const NewEpollInstance &src);
NewEpollInstance::NewEpollInstance(const NewEpollInstance &src) {
    *this = src;
}

// NewEpollInstance &operator=(const NewEpollInstance &src);
NewEpollInstance &NewEpollInstance::operator=(const NewEpollInstance &src) {
    if (this != &src) {
        this->_epollFd = src._epollFd;
        this->_configEpollEvents = src._configEpollEvents;
        for (int i = 0; i < MAX_EVENTS; i++) {
            this->_readyList[i].data.fd = src._readyList[i].data.fd;
            this->_readyList[i].events = src._readyList[i].events;
        }
    }
    return (*this);
}

// ~NewEpollInstance(void);
NewEpollInstance::~NewEpollInstance(void) {

}

// void initEpollInstance(void);
void NewEpollInstance::initEpollInstance(void) {
    this->_epollFd = epoll_create(1);
    if (this->_epollFd == -1) {
        //throw()
    }
}

const char * EpollInstance::CannotInitEpollInstance::what() const throw() {
    return ("Error: error in creating epoll instance with epoll_create().");
}
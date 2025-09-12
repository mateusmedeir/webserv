#include "EpollInstance.hpp"

// EpollInstance(void);
EpollInstance::EpollInstance(void) {}

// EpollInstance(const EpollInstance &src);
EpollInstance::EpollInstance(const EpollInstance &src) {
    *this = src;
}

// EpollInstance &operator=(const EpollInstance &src);
EpollInstance &EpollInstance::operator=(const EpollInstance &src) {
    if (this != &src) {
        this->_epollFd = src._epollFd;
        this->_configEpollEvents = src._configEpollEvents;
        for (int i = 0; i < MAX_EVENTS; i++) {
            this->_readyList[i] = src._readyList[i];
        }
    }
    return (*this);
}

// ~EpollInstance(void);
EpollInstance::~EpollInstance(void) {
    // Nao sei se esse loop para dar close nos fds da readylist e, de fato, necessario.
    // for (int i = 0; i < MAX_EVENTS; i++) {
    //     close(this->_readyList[i].data.fd);
    // }
    close(this->_epollFd);
}

// int getEpollFd(void) const;
int EpollInstance::getEpollFd(void) const {
    return (this->_epollFd);
}

// void initEpollInstance(void);
void EpollInstance::initEpollInstance(void) {
    this->_epollFd = epoll_create(1);
    if (this->_epollFd == -1) {
        throw(EpollInstance::CannotInitEpollInstance());
    }
}

// void setConfigEpollEvents(uint32_t events, int socketFd);
void EpollInstance::setConfigEpollEvents(uint32_t events, int socketFd) {
    this->_configEpollEvents.events = events;
    this->_configEpollEvents.data.fd = socketFd;
}



const char * EpollInstance::CannotInitEpollInstance::what() const throw() {
    return ("Error: error in creating epoll instance with epoll_create().");
}

const char * EpollInstance::CannotManipulateEpollInstance::what() const throw() {
    return ("Error: error in manipulating the epoll instance with epoll_ctl().");
}
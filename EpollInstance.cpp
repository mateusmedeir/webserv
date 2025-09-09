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
    close(this->_epollFd);
    // Nao sei se esse loop para dar close nos fds da readylist e, de fato, necessario.
    // for (int i = 0; i < MAX_EVENTS; i++) {
    //     close(this->_readyList[i].data.fd);
    // }
}

// int getEpollFd(void) const;
int EpollInstance::getEpollFd(void) const {
    return (this->_epollFd);
}

// void manipulateEpollInstance(int operation, int socketFd);
void EpollInstance::manipulateEpollInstance(int operation, int socketFd) {
    if (operation != EPOLL_CTL_ADD && operation != EPOLL_CTL_MOD && operation != EPOLL_CTL_DEL) {
        throw(EpollInstance::CannotManipulateEpollInstance());
    }
    if (epoll_ctl(getEpollFd(), operation, socketFd, &this->_configEpollEvents) == -1) {
        throw(EpollInstance::CannotManipulateEpollInstance());
    }
}

const char * EpollInstance::CannotInitEpollInstance::what() const throw() {
    return ("Error: error in creating epoll instance with epoll_create().");
}

const char * EpollInstance::CannotManipulateEpollInstance::what() const throw() {
    return ("Error: error in manipulating the epoll instance with epoll_ctl().");
}
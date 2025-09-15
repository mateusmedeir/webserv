#include "WebservHeader.hpp"
#include "NewRunTime.hpp"

// NewRunTime(void);
NewRunTime::NewRunTime(void) {
    std::cout << "New Run time got created." << std::endl;
    // this->_epollInstance = NewEpollInstance();
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
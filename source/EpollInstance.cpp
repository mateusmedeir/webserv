#include "../includes/WebservHeader.hpp"
#include <cstring>

EpollInstance::EpollInstance(void) {
    std::cout << "New instance of epoll got created." << std::endl;
    try {
        initEpollInstance();
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

EpollInstance::EpollInstance(const EpollInstance &src) {
    *this = src;
}

EpollInstance::~EpollInstance(void) {
    close(this->_epollFd);
}

EpollInstance &EpollInstance::operator=(const EpollInstance &src) {
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

void EpollInstance::initEpollInstance(void) {
    this->_epollFd = epoll_create(1);
    if (this->_epollFd == -1) {
        throw(EpollInstance::CannotInitEpollInstance());
    }
    std::cout << "Epoll created!" << std::endl;
}

struct epoll_event &EpollInstance::getElementFromReadyList(int index) {
    return (this->_readyList[index]);
}

void EpollInstance::manipInterestList(int operation, EpollHandler *handler) {
    if (operation != EPOLL_CTL_ADD && operation != EPOLL_CTL_DEL && operation != EPOLL_CTL_MOD) {
        throw(EpollInstance::CannotManipulateEpollInstance());
    }
    struct epoll_event data;
    data.events = handler->getInterestedEvents();

    std::cout << "Dentro do manipInterest. FD: " << handler->getSocketFd() << std::endl;

    data.data.ptr = handler;
    if (epoll_ctl(this->_epollFd, operation, handler->getSocketFd(), &data) == -1) {
        std::cerr << "epoll_ctl failed: op=" << operation << " fd=" << handler->getSocketFd()
              << " errno=" << errno << " (" << strerror(errno) << ")\n";
        throw(EpollInstance::CannotManipulateEpollInstance());
    }
    std::cout << "Manipulacao feita com sucesso!" << std::endl;
}

int EpollInstance::manipEpollWait(void) {
    int numberOfReadyFds = 0;
    numberOfReadyFds = epoll_wait(this->_epollFd, this->_readyList, MAX_EVENTS, -1);
    return (numberOfReadyFds);
}


int EpollInstance::getEpollFd(void) const {
    return (this->_epollFd);
}

struct epoll_event EpollInstance::getConfigEpollEvents(void) const {
    return (this->_configEpollEvents);
}

struct epoll_event &EpollInstance::getReadyList(void) {
    return (*this->_readyList);
}

const char * EpollInstance::CannotInitEpollInstance::what() const throw() {
    return ("Error: error in creating epoll instance with epoll_create().");
}

const char * EpollInstance::CannotManipulateEpollInstance::what() const throw() {
    return ("Error: error in controling the epoll instance with epoll_ctl().");
}
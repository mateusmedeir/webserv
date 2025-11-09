#include "../includes/WebservHeader.hpp"
#include <cstring>

EpollInstance *EpollInstance::_instance = NULL;

EpollInstance::EpollInstance(void) {}

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

void EpollInstance::initializeInstance(void) {
    if (_instance == NULL) {
        _instance = new EpollInstance();
        _instance->_epollFd = epoll_create(1);
        if (_instance->_epollFd == -1) {
            throw(EpollInstance::CannotInitEpollInstance());
        }
        std::cout << "Epoll created!" << std::endl;
    }
}

void EpollInstance::deleteInstance(void) {
    if (_instance != NULL) {
        delete _instance;
        _instance = NULL;
    }
}

struct epoll_event &EpollInstance::getElementFromReadyList(int index) {
    if (_instance == NULL) {
        throw std::runtime_error("EpollInstance is not initialized.");
    }

    return (_instance->_readyList[index]);
}

void EpollInstance::manipInterestList(int operation, EpollHandler *handler) {
    if (_instance == NULL) {
        throw std::runtime_error("EpollInstance is not initialized.");
    }

    if (operation != EPOLL_CTL_ADD && operation != EPOLL_CTL_DEL && operation != EPOLL_CTL_MOD) {
        throw(EpollInstance::CannotManipulateEpollInstance());
    }
    struct epoll_event data;
    data.events = handler->getInterestedEvents();

    std::cout << "Dentro do manipInterest. FD: " << handler->getSocketFd() << std::endl;

    data.data.ptr = handler;
    if (operation == EPOLL_CTL_ADD) {
        _instance->_handlers[handler->getSocketFd()] = handler;
    }
    if (epoll_ctl(_instance->_epollFd, operation, handler->getSocketFd(), &data) == -1) {
        std::cerr << "epoll_ctl failed: op=" << operation << " fd=" << handler->getSocketFd()
              << " errno=" << errno << " (" << strerror(errno) << ")\n";
        throw(EpollInstance::CannotManipulateEpollInstance());
    }
    std::cout << "Manipulacao feita com sucesso!" << std::endl;
}

int EpollInstance::manipEpollWait(void) {
    if (_instance == NULL) {
        throw std::runtime_error("EpollInstance is not initialized.");
    }

    int numberOfReadyFds = 0;
    numberOfReadyFds = epoll_wait(_instance->_epollFd, _instance->_readyList, MAX_EVENTS, -1);
    return (numberOfReadyFds);
}

void EpollInstance::deleteElementFromHandlers(int socketFd) {
    if (_instance == NULL) {
        throw std::runtime_error("EpollInstance is not initialized.");
    }

    std::map<int, EpollHandler*>::iterator it = _instance->_handlers.find(socketFd);
    if (it != _instance->_handlers.end()) {
        it->second->deleteHandler();
        _instance->_handlers.erase(it);
    }
}

int EpollInstance::getEpollFd(void) {
    if (_instance == NULL) {
        throw std::runtime_error("EpollInstance is not initialized.");
    }

    return (_instance->_epollFd);
}

struct epoll_event EpollInstance::getConfigEpollEvents(void) {
    if (_instance == NULL) {
        throw std::runtime_error("EpollInstance is not initialized.");
    }

    return (_instance->_configEpollEvents);
}

std::map<int, EpollHandler*> &EpollInstance::getHandlers(void) {
    if (_instance == NULL) {
        throw std::runtime_error("EpollInstance is not initialized.");
    }

    return (_instance->_handlers);
}

struct epoll_event &EpollInstance::getReadyList(void) {
    if (_instance == NULL) {
        throw std::runtime_error("EpollInstance is not initialized.");
    }

    return (*_instance->_readyList);
}

const char * EpollInstance::CannotInitEpollInstance::what() const throw() {
    return ("Error: error in creating epoll instance with epoll_create().");
}

const char * EpollInstance::CannotManipulateEpollInstance::what() const throw() {
    return ("Error: error in controling the epoll instance with epoll_ctl().");
}
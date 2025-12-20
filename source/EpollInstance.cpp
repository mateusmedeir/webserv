#include "../includes/WebservHeader.hpp"
#include <cstring>

EpollInstance *EpollInstance::_instance = NULL;

EpollInstance::EpollInstance(void) {}

EpollInstance::EpollInstance(const EpollInstance &src) {
    *this = src;
}

EpollInstance::~EpollInstance(void) {
    close(this->_epollFd);
    for (std::map<int, EpollHandler*>::iterator it = this->_handlers.begin(); it != this->_handlers.end(); ++it) {
        delete it->second;
    }
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

    int socketFd = handler->getSocketFd();

    if (operation != EPOLL_CTL_ADD && operation != EPOLL_CTL_DEL && operation != EPOLL_CTL_MOD) {
        throw(EpollInstance::CannotManipulateEpollInstance());
    }
    struct epoll_event data;
    data.events = handler->getInterestedEvents();

    data.data.ptr = handler;
    if (operation == EPOLL_CTL_ADD || operation == EPOLL_CTL_MOD) {
        _instance->_handlers[socketFd] = handler;
    } else if (operation == EPOLL_CTL_DEL) {
        _instance->_pendingRemovals.push_back(socketFd);
    }
    if (epoll_ctl(_instance->_epollFd, operation, socketFd, &data) == -1) {
        Logger::debug("EpollInstance::manipInterestList - epoll_ctl failed: op=" + intToString(operation) + " fd=" + intToString(socketFd));
    }
}

void EpollInstance::replaceHandlerFd(EpollHandler *handler, int newFd, uint32_t newEvents) {
    if (_instance == NULL) {
        throw std::runtime_error("EpollInstance is not initialized.");
    }
    if (!handler) {
        throw std::invalid_argument("replaceHandlerFd: handler is null");
    }

    int oldFd = handler->getSocketFd();
    int epfd = _instance->_epollFd;
    if (epfd == -1) {
        throw std::runtime_error("replaceHandlerFd: invalid epoll fd");
    }

    if (newEvents == 0) {
        newEvents = handler->getInterestedEvents();
    }

    if (oldFd == newFd && oldFd != -1) {
        struct epoll_event ev;
        ev.events = newEvents;
        ev.data.ptr = handler;
        if (epoll_ctl(epfd, EPOLL_CTL_MOD, oldFd, &ev) == -1) {
            Logger::debug("replaceHandlerFd: EPOLL_CTL_MOD failed: fd=" + intToString(oldFd));
        }
        handler->setInterestedEvents(newEvents);
        _instance->_handlers[oldFd] = handler;
        return;
    }

    if (oldFd != -1) {
        if (epoll_ctl(epfd, EPOLL_CTL_DEL, oldFd, NULL) == -1) {
            Logger::debug("replaceHandlerFd: EPOLL_CTL_DEL failed for oldFd=" + intToString(oldFd));
        }

        std::map<int, EpollHandler*>::iterator it = _instance->_handlers.find(oldFd);
        if (it != _instance->_handlers.end())
            _instance->_handlers.erase(it);
    }

    struct epoll_event ev;
    ev.events = newEvents;
    ev.data.ptr = handler;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, newFd, &ev) == -1) {
        Logger::debug("replaceHandlerFd: EPOLL_CTL_ADD failed for newFd=" + intToString(newFd));
        handler->setSocketFd(-1);
    }

    handler->setSocketFd(newFd);
    handler->setInterestedEvents(newEvents);
    _instance->_handlers[newFd] = handler;
}


int EpollInstance::manipEpollWait(void) {
    if (_instance == NULL) {
        throw std::runtime_error("EpollInstance is not initialized.");
    }

    int numberOfReadyFds = 0;
    numberOfReadyFds = epoll_wait(_instance->_epollFd, _instance->_readyList, MAX_EVENTS, -1);
    return (numberOfReadyFds);
}

void EpollInstance::deletePendingRemovals() {
    if (_instance == NULL) {
        throw std::runtime_error("EpollInstance is not initialized.");
    }
    for (size_t i = 0; i < _instance->_pendingRemovals.size(); i++) {
        int fd = _instance->_pendingRemovals[i];

        std::map<int, EpollHandler*>::iterator it =
             _instance->_handlers.find(fd);

        if (it != _instance->_handlers.end()) {
            delete it->second;
            _instance->_handlers.erase(it);
        }
    }
    _instance->_pendingRemovals.clear();
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
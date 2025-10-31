#include "../includes/WebservHeader.hpp"

EpollHandler::EpollHandler(uint32_t interestedEvents) : _socketFd(-1), _interestedEvents(interestedEvents) {}

EpollHandler::EpollHandler(int socketFd, uint32_t interestedEvents) : _socketFd(socketFd), _interestedEvents(interestedEvents) {}

EpollHandler::~EpollHandler() {}

int EpollHandler::handleEvent(struct epoll_event &event) {
    // Tratar eventos múltiplos (EPOLLIN | EPOLLOUT podem ocorrer simultaneamente)
    if (event.events & (EPOLLIN | EPOLLRDHUP)) {
        this->handleEpollIn();
    }
    if (event.events & EPOLLOUT) {
        this->handleEpollOut();
    }
    return (0);
}

void EpollHandler::setSocketFd(int socketFd) {
    this->_socketFd = socketFd;
}

int EpollHandler::getSocketFd() const {
    return (this->_socketFd);
}

uint32_t EpollHandler::getInterestedEvents() const {
    return (this->_interestedEvents);
}

void EpollHandler::setInterestedEvents(uint32_t events) {
    this->_interestedEvents = events;
}
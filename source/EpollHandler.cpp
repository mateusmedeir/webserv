#include "../includes/WebservHeader.hpp"

EpollHandler::EpollHandler(uint32_t interestedEvents) : _socketFd(-1), _interestedEvents(interestedEvents) {}

EpollHandler::EpollHandler(int socketFd, uint32_t interestedEvents) : _socketFd(socketFd), _interestedEvents(interestedEvents) {}

EpollHandler::~EpollHandler() {}

int EpollHandler::handleEvent(struct epoll_event &event) {
    switch (event.events) {
        case EPOLLIN:
            this->handleEpollIn();
            break;
        case EPOLLRDHUP:
            this->handleEpollIn();
            break;
        case EPOLLOUT:
            this->handleEpollOut();
            break;
        default:
            return (-1);
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
#include "../includes/WebservHeader.hpp"


EpollHandler::EpollHandler(uint32_t interestedEvents, int socketFd, int maxTimeoutSecs) : _socketFd(socketFd), _interestedEvents(interestedEvents), _maxTimeoutSecs(maxTimeoutSecs), _lastActiveTime(time(NULL)) {}

EpollHandler::~EpollHandler() {}

int EpollHandler::handleEvent(struct epoll_event &event) {
    // Tratar eventos múltiplos (EPOLLIN | EPOLLOUT podem ocorrer simultaneamente)
    if (event.events & (EPOLLIN | EPOLLRDHUP)) {
        this->handleEpollIn();
    }
    if (event.events & EPOLLOUT) {
        this->handleEpollOut();
    }
    // EPOLLHUP indica que o peer fechou a conexão/pipe
    if (event.events & EPOLLHUP) {
        this->handleEpollIn(); // Processar como leitura para detectar EOF
    }
    return (0);
}

void EpollHandler::checkTimeout(void) {
    if (this->_maxTimeoutSecs < 0) {
        return;
    }

    time_t currentTime = time(NULL);
    if (currentTime - this->_lastActiveTime > this->_maxTimeoutSecs) {
        EpollInstance::manipInterestList(EPOLL_CTL_DEL, this);
    }
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

int EpollHandler::getMaxTimeoutSecs() const {
    return (this->_maxTimeoutSecs);
}

void EpollHandler::setInterestedEvents(uint32_t events) {
    this->_interestedEvents = events;
}

void EpollHandler::setMaxTimeoutSecs(int maxTimeoutSecs) {
    this->_maxTimeoutSecs = maxTimeoutSecs;
}

time_t EpollHandler::getLastActiveTime() const {
    return (this->_lastActiveTime);
}

void EpollHandler::setLastActiveTime(time_t lastActiveTime) {
    this->_lastActiveTime = lastActiveTime;
}
#pragma once

#include "WebservHeader.hpp"

class EpollHandler {
    private:
        int _socketFd;
        uint32_t _interestedEvents;
    public:
        EpollHandler(uint32_t interestedEvents);
        EpollHandler(int socketFd, uint32_t interestedEvents);
        virtual ~EpollHandler();

        virtual int handleEvent(struct epoll_event &event);

        virtual void handleEpollIn(void) {};
        virtual void handleEpollOut(void) {};

        virtual void setSocketFd(int socketFd);
        virtual int getSocketFd() const;
        virtual uint32_t getInterestedEvents() const;
};
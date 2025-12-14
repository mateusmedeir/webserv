#pragma once

#include "WebservHeader.hpp"

class EpollHandler {
    private:
        int         _socketFd;
        uint32_t    _interestedEvents;
        int         _maxTimeoutSecs;
        time_t     _lastActiveTime;

    public:
        EpollHandler(uint32_t interestedEvents, int socketFd=-1, int maxTimeoutSecs=-1);
        virtual ~EpollHandler();

        int handleEvent(struct epoll_event &event);
        void checkTimeout(void);

        virtual void handleEpollIn(void) {};
        virtual void handleEpollOut(void) {};

        virtual void setSocketFd(int socketFd);
        virtual int getSocketFd() const;
        virtual uint32_t getInterestedEvents() const;
        virtual void setInterestedEvents(uint32_t events);
        virtual int getMaxTimeoutSecs() const;
        virtual void setMaxTimeoutSecs(int maxTimeoutSecs);
        virtual time_t getLastActiveTime() const;
        virtual void setLastActiveTime(time_t lastActiveTime);
};
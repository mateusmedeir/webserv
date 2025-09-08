#pragma once

# include <netinet/in.h>
# include <sys/epoll.h>
# include "WebservHeader.hpp"

class RunTime {
    private:
        int                 _serverFd;
        int                 _epollInstance;
        struct sockaddr_in  _serverAddr;
        struct epoll_event  _configEpollEvents;
        struct epoll_event  _readyList[MAX_EVENTS];

    public:
        RunTime(void);
        ~RunTime(void);
        RunTime(const RunTime &src);
        RunTime &operator=(const RunTime &src);

        RunTime(int socketDomain, int socketType, int serverPort, int serverAddr);
};
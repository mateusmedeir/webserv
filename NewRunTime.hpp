#pragma once

# include "WebservHeader.hpp"

class NewRunTime {
    private:
        int                 _serverFd;
        struct sockaddr_in  _serverAddr;
        NewEpollInstance    _epollInstance;
    public:
        NewRunTime(void);
        NewRunTime(const NewRunTime &src);
        NewRunTime &operator=(const NewRunTime &src);

        ~NewRunTime(void);
};
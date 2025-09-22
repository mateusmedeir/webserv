#pragma once

# include "WebservHeader.hpp"

class RunTime {
    private:
    public:
        ServerInstance  _server;
        EpollInstance   _epoll;
        
        RunTime(void);
        RunTime(int socketDomain, int socketType);
        RunTime(const RunTime &src);
        RunTime &operator=(const RunTime &src);

        ~RunTime(void);
};
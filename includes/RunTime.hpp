#pragma once

# include "WebservHeader.hpp"
# include <map>


class RunTime {
    private:
    public:
        ServerInstance              _server;
        EpollInstance               _epoll;
        std::map <int, Client> _clients;
        
        RunTime(void);
        RunTime(int socketDomain, int socketType);
        RunTime(const RunTime &src);
        RunTime &operator=(const RunTime &src);

        ~RunTime(void);
};
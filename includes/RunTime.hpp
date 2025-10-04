#pragma once

# include "WebservHeader.hpp"

class RunTime {
    private:
    public:
        ConfigFile                  _config;
        ServerInstance              _server;
        EpollInstance               _epoll;
        std::map <int, ClientState> _clients;
        
        RunTime(void);
        RunTime(int socketDomain, int socketType);
        RunTime(const RunTime &src);
        RunTime &operator=(const RunTime &src);

        ~RunTime(void);
};
#pragma once

# include "WebservHeader.hpp"

class RunTime {
    private:
    public:
        ConfigFile                  _config;
        // ServerInstance              _server;
        EpollInstance               _epoll;
        std::map <int, Client>      _clients;
        
        RunTime(void);
        // RunTime(int socketDomain, int socketType); //APAGAR??
        RunTime(char **av, int ac);
        RunTime(const RunTime &src);
        RunTime &operator=(const RunTime &src);

        ~RunTime(void);

        void deleteClient(int clientFd);
};
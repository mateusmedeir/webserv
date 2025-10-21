#pragma once

# include "WebservHeader.hpp"

class RunTime {
    private:
    public:
        ConfigFile                  config;
        EpollInstance               epoll;
        std::map <int, Client>      clients;
        std::vector<ServerListen>   serverListeners;
        
        RunTime(void);
        RunTime(char **av, int ac);
        RunTime(const RunTime &src);
        RunTime &operator=(const RunTime &src);

        ~RunTime(void);

        void loadServerListeners(void);
        void initServerSockets(int socketDomain, int socketType);
        void deleteClient(int clientFd);

        std::vector<ServerListen> getServerListeners(void) const;
        ServerListen &getElementInServerList(int serverSocketFd);
};
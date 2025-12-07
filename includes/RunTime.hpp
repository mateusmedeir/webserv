#pragma once

# include "WebservHeader.hpp"

class RunTime {
    private:
        static RunTime              *_instance;
        ConfigFile                  _config;
        std::map <int, Client>      _clients;
        std::vector<ServerListen>   _serverListeners;

        RunTime(void);
        RunTime(int ac, char **av);
        RunTime(const RunTime &src);
        RunTime &operator=(const RunTime &src);

    public:
        ~RunTime(void);

        static void initializeRuntime(int ac, char **av);
        static void deleteInstance(void);

        static void loadServerListeners(void);
        static void initServerSockets(int socketDomain, int socketType);
        static void deleteClient(int clientFd);

        static ServerListen &getElementInServerList(int serverSocketFd);
        
        static RunTime &getInstance(void);
        static ConfigFile &getConfig(void);
        static Client &getClient(int clientFd);
        static std::map<int, Client> &getClients(void);
        static std::vector<ServerListen> &getServerListeners(void);
};
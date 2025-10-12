#pragma once

# include "WebservHeader.hpp"

class ServerListen {
    private:
        unsigned int    _host;
        int             _port;
        ServerBlock     _serverBlock;
    public:
        ServerListen(void);
        ServerListen(unsigned int host, int port, ServerBlock &serverBlock);
        ServerListen(const ServerListen &src);
        ServerListen &operator=(const ServerListen &src);
        bool operator==(const ServerListen &other) const;
        ~ServerListen(void);

        unsigned int getHost(void) const;
        int getPort(void) const;
        ServerBlock getServerBlock(void) const;
};
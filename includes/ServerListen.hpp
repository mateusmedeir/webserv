#pragma once

# include "WebservHeader.hpp"

class ServerListen : public EpollHandler {
    private:
        unsigned int        _host;
        int                 _port;
        struct sockaddr_in  _serverAddr;
        const ServerBlock   &_serverBlock;
    public:
        ServerListen(unsigned int host, int port, const ServerBlock &serverBlock);
        ServerListen(const ServerListen &src);
        ServerListen &operator=(const ServerListen &src);
        bool operator==(const ServerListen &other) const;
        ~ServerListen(void);

        virtual void handleEpollIn(void);

        unsigned int getHost(void) const;
        int getPort(void) const;
        ServerBlock getServerBlock(void) const;

        void setServerAddr(int socketDomain);
        void createServerSocket(int socketDomain, int socketType);
        void bindServerSocket(void);
        void allowAddrReuse(void);
        void updateToNonBlocking(void);
        void listenServerSocket(void);
        void initServerSocket(int socketDomain, int socketType);

        class CannotInitServerSocket : public std::exception {
            public:
                virtual const char *what() const throw();
        };

        class CannotBindServerSocket : public std::exception {
            public:
                virtual const char *what() const throw();
        };

        class CannotUpdateServerToNonBlocking : public std::exception {
            public:
                virtual const char *what() const throw();
        };

        class CannotSetServerToListen : public std::exception {
            public:
                virtual const char *what() const throw();
        };

        class CannotAllowAddrReuse : public std::exception {
            public:
                virtual const char *what() const throw();
        };
};
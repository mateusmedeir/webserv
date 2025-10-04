#pragma once

# include "WebservHeader.hpp"

class ServerInstance {
    private:
        int                 _serverFd;
        struct sockaddr_in  _serverAddr;
    public:
        ServerInstance(void);
        ServerInstance(int socketDomain, int socketType);
        ServerInstance(const ServerInstance &src);
        ServerInstance &operator=(const ServerInstance &src);

        ~ServerInstance(void);

        int getServerFd(void) const;

        void setServerAddr(int socketDomain, int serverPort, int serverAddr);

        void initServerSocket(int socketDomain, int socketType);
        void bindServerSocket(void);
        void updateToNonBlocking(void);
        void listenServerSocket(void);

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
};
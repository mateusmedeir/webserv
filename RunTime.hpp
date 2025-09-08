#pragma once

# include "WebservHeader.hpp"
# include <exception>

class RunTime {
    private:
        int                 _serverFd;
        // int                 _epollInstance; //Precisamos criar a classe do epoll antes.
        struct sockaddr_in  _serverAddr;
        // struct epoll_event  _configEpollEvents; //Vamos usar na classe da instancia de Epoll
        // struct epoll_event  _readyList[MAX_EVENTS]; //Vamos usar na classe da instancia de Epoll
    public:
        RunTime(void);
        ~RunTime(void);
        RunTime(const RunTime &src);
        RunTime &operator=(const RunTime &src);

        // RunTime(int socketDomain, int socketType, int serverPort, int serverAddr);

        int getServerFd(void) const;
        struct sockaddr_in getServerAddr(void) const;

        void setServerAddrStruct(int socketDomain, int serverPort, int serverAddr);

        void initServerSocket(int socketDomain, int socketType);
        void bindServerSocket(void);
        void updateToNonBlocking(void);
        void listenServerSocket(void);

        // class GradeTooLowException : public std::exception {
        //     public:
        //         virtual const char *what() const throw();
        // };

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
};

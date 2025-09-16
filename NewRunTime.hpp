#pragma once

# include "WebservHeader.hpp"

class NewEpollInstance;

class NewRunTime : public NewEpollInstance {
    private:
        int                 _serverFd;
        struct sockaddr_in  _serverAddr;
    public:
        NewRunTime(void);
        NewRunTime(int socketDomain, int socketType);
        NewRunTime(const NewRunTime &src);
        NewRunTime &operator=(const NewRunTime &src);

        ~NewRunTime(void);

        virtual void manipInterestList(int operation, uint32_t events, int socketFd);
        virtual int manipEpollWait(void);

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
#pragma once

# include "WebservHeader.hpp"

class EpollInstance {
    private:
        static EpollInstance            *_instance;
        int                             _epollFd;
        struct epoll_event              _configEpollEvents;
        std::map<int, EpollHandler*>    _handlers;
        struct epoll_event              _readyList[MAX_EVENTS];

        EpollInstance(void);
        EpollInstance(const EpollInstance &src);
        EpollInstance &operator=(const EpollInstance &src);
    public:
        ~EpollInstance(void);

        static void initializeInstance(void);
        static void deleteInstance(void);

        static void manipInterestList(int operation, EpollHandler *handler);
        static int manipEpollWait(void);
        static void deleteElementFromHandlers(int socketFd);

        static int getEpollFd(void);
        static struct epoll_event getConfigEpollEvents(void);
        static std::map<int, EpollHandler*> &getHandlers(void);
        static struct epoll_event &getReadyList(void);
        static struct epoll_event &getElementFromReadyList(int index);

        static void setConfigEpollEvents(int socketFd, uint32_t events, bool isServerSocket);
        class CannotInitEpollInstance : public std::exception {
            public:
                virtual const char *what() const throw();
        };

        class CannotManipulateEpollInstance : public std::exception {
            public:
                virtual const char *what() const throw();
        };
};
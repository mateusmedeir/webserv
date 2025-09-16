#pragma once

# include "WebservHeader.hpp"

class NewEpollInstance {
    protected:
        int                 _epollFd;
        struct epoll_event  _configEpollEvents;
        struct epoll_event  _readyList[MAX_EVENTS];
    public:
        NewEpollInstance(void);
        NewEpollInstance(const NewEpollInstance &src);
        NewEpollInstance &operator=(const NewEpollInstance &src);

        virtual ~NewEpollInstance(void);

        void initEpollInstance(void);

        int getEpollFd(void) const;
        struct epoll_event getConfigEpollEvents(void) const;
        struct epoll_event &getReadyList(void);

        void setConfigEpollEvents(int socketFd, uint32_t events);

        struct epoll_event &getElementFromReadyList(int index);

        virtual void manipInterestList(int operation, uint32_t events, int socketFd) = 0;
        virtual int manipEpollWait(void) = 0;

        class CannotInitEpollInstance : public std::exception {
            public:
                virtual const char *what() const throw();
        };

        class CannotManipulateEpollInstance : public std::exception {
            public:
                virtual const char *what() const throw();
        };
};
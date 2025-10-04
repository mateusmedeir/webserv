#pragma once

# include "WebservHeader.hpp"

class EpollInstance {
    private:
        int                 _epollFd;
        struct epoll_event  _configEpollEvents;
        struct epoll_event  _readyList[MAX_EVENTS];
    public:
        EpollInstance(void);
        EpollInstance(const EpollInstance &src);
        EpollInstance &operator=(const EpollInstance &src);

        ~EpollInstance(void);
        
        int getEpollFd(void) const;
        
        void initEpollInstance(void);
        struct epoll_event getConfigEpollEvents(void) const;
        struct epoll_event &getReadyList(void);

        void setConfigEpollEvents(int socketFd, uint32_t events);

        struct epoll_event &getElementFromReadyList(int index);

        void manipInterestList(int operation, uint32_t events, int socketFd);
        int manipEpollWait(void);

        class CannotInitEpollInstance : public std::exception {
            public:
                virtual const char *what() const throw();
        };

        class CannotManipulateEpollInstance : public std::exception {
            public:
                virtual const char *what() const throw();
        };
};
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

        void manipulateEpollInstance(int operation, int socketFd);

        class CannotInitEpollInstance : public std::exception {
            public:
                virtual const char *what() const throw();
        };

        class CannotManipulateEpollInstance : public std::exception {
            public:
                virtual const char *what() const throw();
        };
};
#pragma once

# include "WebservHeader.hpp"

class NewEpollInstance {
    private:
        int                 _epollFd;
        struct epoll_event  _configEpollEvents;
        struct epoll_event  _readyList[MAX_EVENTS];
    public:
        NewEpollInstance(void);
        NewEpollInstance(const NewEpollInstance &src);
        NewEpollInstance &operator=(const NewEpollInstance &src);

        ~NewEpollInstance(void);

        void initEpollInstance(void);

        class CannotInitEpollInstance : public std::exception {
            public:
                virtual const char *what() const throw();
        };
};
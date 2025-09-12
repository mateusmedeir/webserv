#pragma once

# include "WebservHeader.hpp"

// Acho que uma saida e declarar essa classe como Abstrata. Dessa forma, a classe RunTime
// consegue acessar os atributos da Epoll diretamente, sem passar pelos metodos, e vai
// conseguir implementar os metodos necessarios para manipular o Epoll tambem.
// Preciso de ajuda para decidir isso. Mas acho que e uma boa saida.

class EpollInstance {
    protected:
        int                 _epollFd;
        struct epoll_event  _configEpollEvents;
        struct epoll_event  _readyList[MAX_EVENTS];
    public:
        EpollInstance(void);
        EpollInstance(const EpollInstance &src);
        EpollInstance &operator=(const EpollInstance &src);

        virtual ~EpollInstance(void);

        int getEpollFd(void) const;
        void initEpollInstance(void);

        void setConfigEpollEvents(uint32_t events, int socketFd);

        virtual void manipulateEpollInstance(int operation, uint32_t events, int socketFd) const = 0;
        // virtual void initEpollInstance(void) const = 0;

        class CannotInitEpollInstance : public std::exception {
            public:
                virtual const char *what() const throw();
        };

        class CannotManipulateEpollInstance : public std::exception {
            public:
                virtual const char *what() const throw();
        };
};
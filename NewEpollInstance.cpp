#include "WebservHeader.hpp"

// NewEpollInstance(void);
NewEpollInstance::NewEpollInstance() {
    //chamar metodo que via criar o epoll com o epollCreate()
    std::cout << "New instance of epoll got created." << std::endl;
    try
    {
        initEpollInstance();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}

// NewEpollInstance(const NewEpollInstance &src);
NewEpollInstance::NewEpollInstance(const NewEpollInstance &src) {
    *this = src;
}

// NewEpollInstance &operator=(const NewEpollInstance &src);
NewEpollInstance &NewEpollInstance::operator=(const NewEpollInstance &src) {
    if (this != &src) {
        this->_epollFd = src._epollFd;
        this->_configEpollEvents = src._configEpollEvents;
        for (int i = 0; i < MAX_EVENTS; i++) {
            this->_readyList[i].data.fd = src._readyList[i].data.fd;
            this->_readyList[i].events = src._readyList[i].events;
        }
    }
    return (*this);
}

// ~NewEpollInstance(void);
NewEpollInstance::~NewEpollInstance(void) {
    close(this->_epollFd);
}

// void initEpollInstance(void);
void NewEpollInstance::initEpollInstance(void) {
    this->_epollFd = epoll_create(1);
    if (this->_epollFd == -1) {
        throw(NewEpollInstance::CannotInitEpollInstance());
    }
    std::cout << "Epoll created!" << std::endl;
}

// int getEpollFd(void) const;
int NewEpollInstance::getEpollFd(void) const {
    return (this->_epollFd);
}

// struct epoll_event getConfigEpollEvents(void) const;
struct epoll_event NewEpollInstance::getConfigEpollEvents(void) const {
    return (this->_configEpollEvents);
}

// struct epoll_event getReadyList(void) const;
struct epoll_event &NewEpollInstance::getReadyList(void) {
    return (*this->_readyList);
}

void NewEpollInstance::setConfigEpollEvents(int socketFd, uint32_t events) {
    this->_configEpollEvents.data.fd = socketFd;
    this->_configEpollEvents.events = events;
}

const char * NewEpollInstance::CannotInitEpollInstance::what() const throw() {
    return ("Error: error in creating epoll instance with epoll_create().");
}

const char * NewEpollInstance::CannotManipulateEpollInstance::what() const throw() {
    return ("Error: error in controling the epoll instance with epoll_ctl().");
}
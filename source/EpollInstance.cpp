#include "../includes/WebservHeader.hpp"

EpollInstance::EpollInstance(void) {
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

EpollInstance::EpollInstance(const EpollInstance &src) {
    *this = src;
}

EpollInstance &EpollInstance::operator=(const EpollInstance &src) {
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

EpollInstance::~EpollInstance(void) {
    close(this->_epollFd);
}

void EpollInstance::initEpollInstance(void) {
    this->_epollFd = epoll_create(1);
    if (this->_epollFd == -1) {
        throw(EpollInstance::CannotInitEpollInstance());
    }
    std::cout << "Epoll created!" << std::endl;
}

int EpollInstance::getEpollFd(void) const {
    return (this->_epollFd);
}

struct epoll_event EpollInstance::getConfigEpollEvents(void) const {
    return (this->_configEpollEvents);
}

struct epoll_event &EpollInstance::getReadyList(void) {
    return (*this->_readyList);
}

void EpollInstance::setConfigEpollEvents(int socketFd, uint32_t events, bool isServerSocket) {
    struct epoll_event aux;

    aux.data.fd = socketFd;
    if (isServerSocket) aux.data.ptr = (bool *)isServerSocket, aux.data.ptr = NULL;
    aux.events = events;
    (void)aux;
    // this->_configEpollEvents.data.fd = socketFd;
    // if (isServerSocket) {
    //     std::cout << "Modificamos o data.ptr para identificar que e um socket de servidor." << std::endl;
    //     this->_configEpollEvents.data.ptr = (bool *)isServerSocket;
    // }
    // else {
    //     this->_configEpollEvents.data.ptr = NULL;
    // }
    // this->_configEpollEvents.events = events;
}

struct epoll_event &EpollInstance::getElementFromReadyList(int index) {
    struct epollUserData *aux = (struct epollUserData *)this->_readyList[index].data.ptr;
    std::cout << "Dentro da ready list: " << aux->fd << std::endl;
    return (this->_readyList[index]);
}

void EpollInstance::manipInterestList(int operation, uint32_t events, int socketFd, bool isServerSocket) {
    if (operation != EPOLL_CTL_ADD && operation != EPOLL_CTL_DEL && operation != EPOLL_CTL_MOD) {
        throw(EpollInstance::CannotManipulateEpollInstance());
    }
    struct epoll_event aux;
    struct epollUserData *data = new struct epollUserData;
    data->fd = socketFd;
    data->isServerSocket = isServerSocket;

    std::cout << "Dentro do manipInterest. FD: " << socketFd << std::endl;

    aux.data.ptr = data;
    aux.events = events;
    // this->setConfigEpollEvents(socketFd, events, isServerSocket);
    if (epoll_ctl(this->_epollFd, operation, socketFd, &aux) == -1) {
        throw(EpollInstance::CannotManipulateEpollInstance());
    }
    std::cout << "Manipulacao feita com sucesso!" << std::endl;
}

int EpollInstance::manipEpollWait(void) {
    int numberOfReadyFds = 0;
    numberOfReadyFds = epoll_wait(this->_epollFd, this->_readyList, MAX_EVENTS, 0);
    return (numberOfReadyFds);
}

const char * EpollInstance::CannotInitEpollInstance::what() const throw() {
    return ("Error: error in creating epoll instance with epoll_create().");
}

const char * EpollInstance::CannotManipulateEpollInstance::what() const throw() {
    return ("Error: error in controling the epoll instance with epoll_ctl().");
}
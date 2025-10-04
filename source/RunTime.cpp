# include "../includes/RunTime.hpp"

RunTime::RunTime(void) {}

RunTime::RunTime(int socketDomain, int socketType): _server(socketDomain, socketType), _epoll() {
    std::cout << "RunTime got created!" << std::endl;
}

RunTime::RunTime(const RunTime &src) {
    *this = src;
}

RunTime &RunTime::operator=(const RunTime &src) {
    if (this != &src) {
        this->_server = src._server;
        this->_epoll = src._epoll;
    }
    return (*this);
}

RunTime::~RunTime(void) {}

void RunTime::deleteClient(int clientFd) {
    this->_clients.erase(clientFd);
    close(clientFd);
}
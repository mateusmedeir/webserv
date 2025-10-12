#include "../includes/WebservHeader.hpp"

ServerListen::ServerListen(void) {}

ServerListen::ServerListen(unsigned int host, int port, ServerBlock &serverBlock)
    : _host(host), _port(port), _serverBlock(serverBlock) {}

ServerListen::ServerListen(const ServerListen &src)
    : _host(src._host), _port(src._port), _serverBlock(src._serverBlock) {}

ServerListen &ServerListen::operator=(const ServerListen &src) {
    if (this != &src) {
        this->_host = src._host;
        this->_port = src._port;
        this->_serverBlock = src._serverBlock;
    }
    return (*this);
}

bool ServerListen::operator==(const ServerListen &other) const {
    return (this->_host == other._host && this->_port == other._port);
}

ServerListen::~ServerListen(void) {}

unsigned int ServerListen::getHost(void) const {
    return (this->_host);
}

int ServerListen::getPort(void) const {
    return (this->_port);
}

ServerBlock ServerListen::getServerBlock(void) const {
    return (this->_serverBlock);
}
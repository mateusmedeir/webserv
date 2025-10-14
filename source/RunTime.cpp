# include "../includes/RunTime.hpp"

RunTime::RunTime(void) {}

RunTime::RunTime(int socketDomain, int socketType): _server(socketDomain, socketType), _epoll() {
    std::cout << "RunTime got created!" << std::endl;
}

// RunTime(char **ac, int av);
RunTime::RunTime(char **ac, int av): _config(ac, av) {
    std::cout << "Novo construtor RunTime..." << std::endl;
    std::vector<ServerListen> serverSocketsToCreate = this->_config.getServerListens();

    for (int i = 0; i < serverSocketsToCreate.size(); i++) {
        std::cout << "Host[" << i << "]: " << serverSocketsToCreate[i].getHost() << " Port[" << i << "]: " << serverSocketsToCreate[i].getPort() << std::endl;
        
    }
    std::cout << "Numero de sockets: " << serverSocketsToCreate.size() << std::endl;
    // Loop pelo serverListen e vou criando novos sockets a partir do tamanho do server block?
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
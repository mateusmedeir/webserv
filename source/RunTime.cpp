# include "../includes/RunTime.hpp"

RunTime::RunTime(void) {}

// RunTime::RunTime(int socketDomain, int socketType): _server(socketDomain, socketType), _epoll() {
//     std::cout << "RunTime got created!" << std::endl;
// }

// RunTime(char **ac, int av);
RunTime::RunTime(char **ac, int av): _config(ac, av), _epoll() {
    std::cout << "Novo construtor RunTime..." << std::endl;
    this->_config.initServerSockets(AF_INET, SOCK_STREAM);
    std::vector<ServerListen> serverSocketsToCreate = this->_config.getServerListens();

    std::cout << "Numero de sockets: " << serverSocketsToCreate.size() << std::endl;
    // Loop pelo serverListen e vou criando novos sockets a partir do tamanho do server block?
    for (unsigned int i = 0; i < serverSocketsToCreate.size(); i++) {
        std::cout << "Host[" << i << "]: " << serverSocketsToCreate[i].getHost() << " Port[" << i << "]: " << serverSocketsToCreate[i].getPort() << std::endl;
        //Estamos iniciando todos os sockets que o .conf nos disponibilizou
        // serverSocketsToCreate[i].initServerSockets(AF_INET, SOCK_STREAM);
        //Depois de iniciar, precisamos por o socket em questao na interest list do epoll
        std::cout << "debugging serverfd: " << serverSocketsToCreate[i].getServerFd() << std::endl;
        this->_epoll.manipInterestList(EPOLL_CTL_ADD, EPOLLIN, serverSocketsToCreate[i].getServerFd(), 1);
    }
}

RunTime::RunTime(const RunTime &src) {
    *this = src;
}

RunTime &RunTime::operator=(const RunTime &src) {
    if (this != &src) {
        // this->_server = src._server;
        this->_epoll = src._epoll;
    }
    return (*this);
}

RunTime::~RunTime(void) {}

void RunTime::deleteClient(int clientFd) {
    this->_clients.erase(clientFd);
    close(clientFd);
}
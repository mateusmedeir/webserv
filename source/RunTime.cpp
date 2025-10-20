# include "../includes/RunTime.hpp"

RunTime::RunTime(void) {}

RunTime::RunTime(char **ac, int av): config(ac, av), epoll() {
    std::cout << "Novo construtor RunTime..." << std::endl;

    this->loadServerListeners();
    this->initServerSockets(AF_INET, SOCK_STREAM);
}

RunTime::RunTime(const RunTime &src) {
    *this = src;
}

RunTime &RunTime::operator=(const RunTime &src) {
    if (this != &src) {
        this->epoll = src.epoll;
        this->config = src.config;
    }
    return (*this);
}

RunTime::~RunTime(void) {}

void RunTime::deleteClient(int clientFd) {
    this->clients.erase(clientFd);
    close(clientFd);
}

void RunTime::loadServerListeners(void) {
	std::set<std::pair<unsigned int, int> > uniqueListens;
	for (size_t i = 0; i < this->config.getServerBlocks().size(); i++)
	{
		std::vector<t_listen> listens = this->config.getServerBlocks()[i].getListen();

		for (size_t j = 0; j < listens.size(); j++) {
			std::pair<unsigned int, int> key(listens[j].host, listens[j].port);

			if (uniqueListens.insert(key).second)
				this->serverListeners.push_back(ServerListen(listens[j].host, listens[j].port, this->config.getServerBlocks()[i]));
		}
	}
}

void RunTime::initServerSockets(int socketDomain, int socketType) {
    for (unsigned int i = 0; i < this->serverListeners.size(); i++) {
        this->serverListeners[i].initServerSocket(socketDomain, socketType);
        this->epoll.manipInterestList(EPOLL_CTL_ADD, EPOLLIN, this->serverListeners[i].getServerFd(), 1);
    }
}

std::vector<ServerListen> RunTime::getServerListeners(void) const {
    return (this->serverListeners);
}

ServerListen &RunTime::getElementInServerList(int serverSocketFd) {
    for (size_t i = 0; i < this->serverListeners.size(); i++) {
        if (this->serverListeners[i].getServerFd() == serverSocketFd)
            return (this->serverListeners[i]);
    }
    throw std::runtime_error("ServerListen not found for the given socket FD.");
}
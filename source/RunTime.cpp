# include "../includes/RunTime.hpp"

RunTime *RunTime::_instance = NULL;

RunTime::RunTime(void) {}

RunTime::RunTime(int ac, char **av): _config(ac, av), _epoll() {
    std::cout << "Novo construtor RunTime..." << std::endl;
}

RunTime::RunTime(const RunTime &src) {
    *this = src;
}

RunTime &RunTime::operator=(const RunTime &src) {
    if (this != &src) {
        this->_epoll = src._epoll;
        this->_config = src._config;
    }
    return (*this);
}

RunTime::~RunTime(void) {}

void RunTime::initializeRuntime(int ac, char **av) {
    if (_instance == NULL) {
        _instance = new RunTime(ac, av);
        _instance->loadServerListeners();
        _instance->initServerSockets(AF_INET, SOCK_STREAM);
    }
}

void RunTime::deleteInstance(void) {
    if (_instance != NULL) {
        delete _instance;
        _instance = NULL;
    }
}

void RunTime::deleteClient(int clientFd) {
    if (_instance == NULL) {
        throw std::runtime_error("RunTime instance is not initialized.");
    }
    _instance->_clients.erase(clientFd);
    close(clientFd);
}

void RunTime::loadServerListeners(void) {
    std::cout << "Loading server listeners..." << std::endl;
	std::set<std::pair<unsigned int, int> > uniqueListens;
	for (size_t i = 0; i < _instance->_config.getServerBlocks().size(); i++)
	{
        std::cout << "Loading server block " << i + 1 << "..." << std::endl;
		std::vector<t_listen> listens = _instance->_config.getServerBlocks()[i].getListen();

		for (size_t j = 0; j < listens.size(); j++) {
			std::pair<unsigned int, int> key(listens[j].host, listens[j].port);

			if (uniqueListens.insert(key).second)
				_instance->_serverListeners.push_back(ServerListen(listens[j].host, listens[j].port, _instance->_config.getServerBlocks()[i]));
		}
	}
}

void RunTime::initServerSockets(int socketDomain, int socketType) {
    for (unsigned int i = 0; i < _instance->_serverListeners.size(); i++) {
        _instance->_serverListeners[i].initServerSocket(socketDomain, socketType);
        _instance->_epoll.manipInterestList(EPOLL_CTL_ADD, &_instance->_serverListeners[i]);
    }
}

RunTime &RunTime::getInstance(void) {
    return (*_instance);
}

std::vector<ServerListen> &RunTime::getServerListeners(void) {
    if (_instance == NULL) {
        throw std::runtime_error("RunTime instance is not initialized.");
    }
    return (_instance->_serverListeners);
}

ServerListen &RunTime::getElementInServerList(int serverSocketFd) {
    if (_instance == NULL) {
        throw std::runtime_error("RunTime instance is not initialized.");
    }

    for (size_t i = 0; i < _instance->_serverListeners.size(); i++) {
        if (_instance->_serverListeners[i].getSocketFd() == serverSocketFd)
            return (_instance->_serverListeners[i]);
    }
    throw std::runtime_error("ServerListen not found for the given socket FD.");
}

ConfigFile &RunTime::getConfig(void) {
    if (_instance == NULL) {
        throw std::runtime_error("RunTime instance is not initialized.");
    }
    return (_instance->_config);
}

EpollInstance &RunTime::getEpoll(void) {
    if (_instance == NULL) {
        throw std::runtime_error("RunTime instance is not initialized.");
    }
    return (_instance->_epoll);
}

Client &RunTime::getClient(int clientFd) {
    if (_instance == NULL) {
        throw std::runtime_error("RunTime instance is not initialized.");
    }
    std::map<int, Client>::iterator it = _instance->_clients.find(clientFd);
    if (it == _instance->_clients.end()) {
        throw std::runtime_error("Client not found for the given client FD.");
    }
    return (it->second);
}

std::map<int, Client> &RunTime::getClients(void) {
    if (_instance == NULL) {
        throw std::runtime_error("RunTime instance is not initialized.");
    }
    return (_instance->_clients);
}
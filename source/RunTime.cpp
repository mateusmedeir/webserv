# include "../includes/RunTime.hpp"

RunTime *RunTime::_instance = NULL;

RunTime::RunTime(void) {}

RunTime::RunTime(int ac, char **av): _config(ac, av) {}

RunTime::RunTime(const RunTime &src) {
    *this = src;
}

RunTime &RunTime::operator=(const RunTime &src) {
    if (this != &src) {
        this->_config = src._config;
    }
    return (*this);
}

RunTime::~RunTime(void) {}

void RunTime::initializeRuntime(int ac, char **av) {
    if (_instance == NULL) {
        _instance = new RunTime(ac, av);
        EpollInstance::initializeInstance();
        _instance->initServerSockets();
    }
}

void RunTime::deleteInstance(void) {
    if (_instance != NULL) {
        delete _instance;
        EpollInstance::deleteInstance();
        _instance = NULL;
    }
}

void RunTime::initServerSockets(void) {
	std::set<std::pair<unsigned int, int> > uniqueListens;
	for (size_t i = 0; i < _instance->_config.getServerBlocks().size(); i++)
	{
		std::vector<t_listen> listens = _instance->_config.getServerBlocks()[i].getListen();

		for (size_t j = 0; j < listens.size(); j++) {
			std::pair<unsigned int, int> key(listens[j].host, listens[j].port);

			if (uniqueListens.insert(key).second)
                EpollInstance::manipInterestList(EPOLL_CTL_ADD, new ServerListen(listens[j].host, listens[j].port, _instance->_config.getServerBlocks()[i]));
		}
	}
}

RunTime &RunTime::getInstance(void) {
    return (*_instance);
}

ConfigFile &RunTime::getConfig(void) {
    if (_instance == NULL) {
        throw std::runtime_error("RunTime instance is not initialized.");
    }
    return (_instance->_config);
}
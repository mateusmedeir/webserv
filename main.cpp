#include "includes/RunTime.hpp"

void signalHandler(int signum) {
    if (signum == SIGINT)
        RunTime::deleteInstance();
}

int	verifyArgs(int ac, char **av)
{
	if (ac > 2)
	{
		std::cerr << "Usage: " << av[0] << " <config_file>\n\tOR\nUsage: " << av[0] << std::endl;
		return (0);
	}
	return (1);
}

void	printBlock(std::vector<ServerBlock> serverBlocks, std::vector<ServerListen> serverListens)
{
    for (size_t i = 0; i < serverBlocks.size(); i++)
    {
        std::cout << "==================== SERVER BLOCK " << i + 1 << " ====================" << std::endl;
        serverBlocks[i].printServerBlock();
        std::cout << "Listens: " << std::endl;
        for (size_t j = 0; j < serverListens.size(); j++)
        {
            if (serverListens[j].getServerBlock() == serverBlocks[i])
            {
                std::cout << "Host[" << j << "]: " << serverListens[j].getHost() << " Port[" << j << "]: " << serverListens[j].getPort() << std::endl;
            }
        }
        std::map<std::string, LocationBlock> locations = serverBlocks[i].getLocations();
        for (std::map<std::string, LocationBlock>::iterator it = locations.begin(); it != locations.end(); it++)
        {
            std::cout << "------------------ LOCATION BLOCK ------------------" << std::endl;
            it->second.printLocationBlock();
            std::cout << "----------------------------------------------------" << std::endl;
        }
        std::cout << "========================================================" << std::endl;
    }
}

void epollReadyListLoop(int numberOfReadySockets) {
    if (numberOfReadySockets) {
        for (int i = 0; i < numberOfReadySockets; i++) {
            struct epoll_event &data = RunTime::getEpoll().getElementFromReadyList(i);
            
            EpollHandler *handler = static_cast<EpollHandler *>(data.data.ptr);
            if (handler) {
                handler->handleEvent(data);
            }
        }
    }
}

void serverMainLoop() {
    while (true) {
        int numberOfReadySockets = RunTime::getEpoll().manipEpollWait();
        if (numberOfReadySockets == -1) {
            std::cerr << "Error: erro ao manipular o epoll_wait()." << std::endl;
            break;
        }
        else {
            epollReadyListLoop(numberOfReadySockets);
        }
    }
}

int main(int ac, char **av) {
    if (!verifyArgs(ac, av))
        return (1);

    signal(SIGINT, signalHandler);

    try {
        RunTime::initializeRuntime(ac, av);
        printBlock(RunTime::getConfig().getServerBlocks(), RunTime::getServerListeners());
        serverMainLoop();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        RunTime::deleteInstance();
        return (-1);
    }
    RunTime::deleteInstance();

    return (0);
}

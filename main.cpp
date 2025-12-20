#include "includes/RunTime.hpp"

void signalHandler(int signum) {
    if (signum == SIGINT) {
        Logger::debug("[Signal] SIGINT received, shutting down gracefully...");
        RunTime::deleteInstance();
        Logger::deleteInstance();
    }
    else if (signum == SIGPIPE) {
        Logger::debug("[Signal] SIGPIPE received and ignored (client disconnected during write)");
    }
}

int	verifyArgs(int ac, char **av)
{
	if (ac > 2)
	{
		Logger::error("Usage: " + std::string(av[0]) + " <config_file>\n\tOR\nUsage: " + std::string(av[0]));
		return (0);
	}
	return (1);
}

void epollReadyListLoop(int numberOfReadySockets) {
    if (numberOfReadySockets) {
        for (int i = 0; i < numberOfReadySockets; i++) {
            struct epoll_event &data = EpollInstance::getElementFromReadyList(i);
            
            EpollHandler *handler = static_cast<EpollHandler *>(data.data.ptr);
            if (handler) {
                handler->handleEvent(data);
            }
        }
    }
}

void epollValidationLoop() {
    std::map<int, EpollHandler*> &handlers = EpollInstance::getHandlers();
    for (std::map<int, EpollHandler*>::iterator it = handlers.begin(); it != handlers.end(); ++it) {
        it->second->checkTimeout();
    }
}

void serverMainLoop() {
    while (true) {
        int numberOfReadySockets = EpollInstance::manipEpollWait();
        if (numberOfReadySockets == -1) {
            Logger::error("Error: erro ao manipular o epoll_wait().");
            break;
        }
        else {
            epollReadyListLoop(numberOfReadySockets);
            epollValidationLoop();
            EpollInstance::deletePendingRemovals();
        }
    }
}

int main(int ac, char **av) {
    if (!verifyArgs(ac, av))
        return (1);

    signal(SIGINT, signalHandler);
    signal(SIGPIPE, signalHandler);

    CompositeLogHandler* compositeHandler = new CompositeLogHandler();
    compositeHandler->addHandler(new StdLogHandler());
    compositeHandler->addHandler(new FileLogHandler("application.log"));
    Logger::initLogger(INFO, compositeHandler);

    try {
        RunTime::initializeRuntime(ac, av);
        serverMainLoop();
    } catch (const std::exception &e) {
        Logger::debug("Exception caught in main: " + std::string(e.what()));
        RunTime::deleteInstance();
        Logger::deleteInstance();
        return (-1);
    }
    RunTime::deleteInstance();
    Logger::deleteInstance();

    return (0);
}

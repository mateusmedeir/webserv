#pragma once

# define MAX_EVENTS 10

# include <iostream> //| Para testes, remover depois
# include <sys/socket.h>
# include <sys/stat.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/epoll.h>
# include <cerrno>
# include <exception>
# include <algorithm>
# include <vector>
# include <map>
# include <string>
# include <utility>
# include <cstddef>
# include <cstdlib>
# include <ctime>
# include <cstring>
# include <fstream>
# include <sstream>
# include <cctype>
# include <set>
# include <csignal>

enum    LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

typedef struct s_logEvent {
    enum LogLevel   level;
    std::string     message;
}t_logEvent;

# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "ServerBlock.hpp"
# include "ConfigFile.hpp"
# include "EpollHandler.hpp"
# include "EpollInstance.hpp"
# include "LocationBlock.hpp"
# include "ServerListen.hpp"
# include "Client.hpp"
# include "CookieHandler.hpp"
# include "CgiProcess.hpp"
# include "CgiHandler.hpp"
# include "CgiPipeHandler.hpp"
# include "Logger.hpp"
# include "StdLogHandler.hpp"
# include "FileLogHandler.hpp"
# include "CompositeLogHandler.hpp"

enum clientBufferState {
    READING_HEADER = 9, //Lendo o header da request ainda
    READING_BODY = 10, //Lendo o conteudo da request ainda
    COMPLETE = 11, //Ja lemos todo o conteudo da request
};

// void initAllLogHandlers(void);
void set_nonblocking(int sockfd);
#pragma once

# define MAX_EVENTS 10
# define MAX_BUFFER_SIZE 4096

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
# include <sys/wait.h>
# include <dirent.h>


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
# include "LogHandler.hpp"
# include "StdLogHandler.hpp"
# include "FileLogHandler.hpp"
# include "CompositeLogHandler.hpp"
# include "CgiHandler.hpp"

enum clientBufferState {
    READING_HEADER = 9, //Lendo o header da request ainda
    READING_BODY = 10, //Lendo o conteudo da request ainda
    WAITING_CGI = 11, //Aguardando resposta do CGI
    COMPLETE = 12, //Ja lemos todo o conteudo da request
};

enum cgiBufferState {
    IN_PROGRESS = 25,
    FAILED = 26,
    COMPLETED = 27
};

// void initAllLogHandlers(void);
void setNonBlocking(int sockfd);
std::string intToString(int n);
std::string extractUriWithoutQuery(const std::string &uri);
std::string extractQueryFromUri(const std::string &uri);
std::string extractUriPathInfo(const std::string &uri, const LocationBlock &location);
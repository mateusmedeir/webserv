#pragma once

# define MAX_EVENTS 10

# include <iostream> //| Para testes, remover depois
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/tcp.h>  // Para TCP_NODELAY
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
# include <fstream>
# include <sstream>
# include <cctype>
#include <set>
#include <csignal>


# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "ServerBlock.hpp"
# include "ConfigFile.hpp"
# include "CookieHandler.hpp"
# include "EpollHandler.hpp"
# include "EpollInstance.hpp"
# include "LocationBlock.hpp"
# include "ServerInstance.hpp"
# include "ServerListen.hpp"
# include "Client.hpp"
# include "Cgi.hpp"

enum clientBufferState {
    READING_HEADER = 9,  //Lendo o header da request ainda
    READING_BODY = 10,   //Lendo o conteudo da request ainda
    EXECUTING_CGI = 11,  //Executando script CGI (aguardando)
    COMPLETE = 12,       //Ja lemos todo o conteudo da request
};

void set_nonblocking(int sockfd);
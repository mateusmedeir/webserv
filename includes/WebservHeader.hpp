#pragma once

# define MAX_EVENTS 10

# include <iostream> //| Para testes, remover depois
# include <sys/socket.h>
# include <netinet/in.h>
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

# include "HttpResponse.hpp"
# include "HttpRequest.hpp"
# include "Client.hpp"
# include "ConfigFile.hpp"
# include "EpollInstance.hpp"
# include "LocationBlock.hpp"
# include "ServerBlock.hpp"
# include "ServerInstance.hpp"

enum clientBufferState {
    READING_HEADER = 9, //Lendo o header da request ainda
    READING_BODY = 10, //Lendo o conteudo da request ainda
    COMPLETE = 11, //Ja lemos todo o conteudo da request
};

void set_nonblocking(int sockfd);
#pragma once

# define MAX_EVENTS 10

# include <iostream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/epoll.h>
# include <cerrno>
# include <exception>
# include <algorithm>

# include "EpollInstance.hpp"
# include "ServerInstance.hpp"
# include "Client.hpp"
# include "RunTime.hpp"

enum clientBufferState {
    IN_PROGRESS = 10, //Lendo o conteudo da request ainda
    COMPLETE = 11, //Ja lemos todo o conteudo da request
};

void set_nonblocking(int sockfd);
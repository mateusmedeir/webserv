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

# include "EpollInstance.hpp"
# include "RunTime.hpp"


int set_nonblocking(int sockfd);
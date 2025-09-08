#pragma once

# include <iostream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/epoll.h>
# include <cerrno>

# define MAX_EVENTS 10

int set_nonblocking(int sockfd);
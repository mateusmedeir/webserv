#include "../includes/WebservHeader.hpp"

void set_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        throw(ServerListen::CannotUpdateServerToNonBlocking());
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw(ServerListen::CannotUpdateServerToNonBlocking());
    }
}
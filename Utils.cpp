#include "WebservHeader.hpp"

int set_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        return (std::cerr << "fcntl(F_GETFL)" << std::endl, -1);
        // perror("fcntl(F_GETFL)");
        // return -1;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return (std::cerr << "fcntl(F_SETFL)", -1);
        // perror("fcntl(F_SETFL)");
        // return -1;
    }
    return 0;
}
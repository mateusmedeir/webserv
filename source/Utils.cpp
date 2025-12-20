#include "../includes/WebservHeader.hpp"

void setNonBlocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        throw(ServerListen::CannotUpdateServerToNonBlocking());
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw(ServerListen::CannotUpdateServerToNonBlocking());
    }
}

std::string intToString(int n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

std::string extractUriWithoutQuery(const std::string &uri) {
    size_t pos = uri.find('?');
    if (pos != std::string::npos) {
        return uri.substr(0, pos);
    }
    return uri;
}

std::string extractQueryFromUri(const std::string &uri) {
    size_t pos = uri.find('?');
    if (pos != std::string::npos && pos + 1 < uri.size()) {
        return uri.substr(pos + 1);
    }
    return "";
}

std::string extractUriPathInfo(const std::string &uri, const LocationBlock &location) {
    std::string path = extractUriWithoutQuery(uri);
    size_t locUriLen = location.getUri().length();

    if (path.length() > locUriLen) {
        return path.substr(locUriLen);
    }
    return "";
}

// void initAllLogHandlers(void) {
//     CompositeLogHandler compositeHandler;

//     compositeHandler.addHandler(new StdLogHandler());
//     compositeHandler.addHandler(new FileLogHandler("../application.log"));

//     Logger::initLogger(DEBUG, new CompositeLogHandler());
// }
#include "../includes/WebservHeader.hpp"
#include "../includes/RunTime.hpp"

Client::Client(int clientFd, ServerListen &serverListen) : EpollHandler(clientFd, EPOLLIN), _serverListen(serverListen) {
    std::cout << "Client got created..." << std::endl;
    this->_state = READING_HEADER;
    this->_rawRequest = "";
    this->request = HttpRequest();
    this->response = HttpResponse();
}

Client::Client(const Client &src) : EpollHandler(src.getSocketFd(), src.getInterestedEvents()), _serverListen(src._serverListen) {
    *this = src;
}

Client &Client::operator=(const Client &src) {
    if (this != &src) {
        this->request = src.request;
        this->response = src.response;
        this->_state = src._state;
    }
    return (*this);
}

Client::~Client(void) {}

void Client::handleEpollIn(void) {
    char buffer[5] = {0};
    int count = 0;

    if ((count = read(this->getSocketFd(), buffer, 1)) > 0) {
        this->concatenateRequestData(buffer);
        if (this->isRequestComplete()) {
            std::cout << "================== REQUEST COMPLETE =================" << std::endl;
            std::cout << this->request.getMethod() << std::endl;
            std::cout << this->request.getUri() << std::endl;
            std::map<std::string, std::string> headers = this->request.getHeaders();
            for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++) {
                std::cout << it->first << ": " << it->second << std::endl;
            }
            std::cout << "Body: " << this->request.getBody() << std::endl;
            std::cout << "=====================================================" << std::endl;
            this->response.dispatchRequest(this->request);
            std::string responseStr = this->response.toString();
            std::cout << "=================== RESPONSE SEND ===================" << std::endl;
            std::cout << responseStr << std::endl;
            std::cout << "=====================================================" << std::endl;
            send(this->getSocketFd(), responseStr.c_str(), responseStr.size(), 0);
            RunTime::deleteClient(this->getSocketFd());
        }
    } else if (count == 0) {
        std::cout << "Client closed the connection." << std::endl;
        std::cout << this->getRawRequest() << std::endl;
        RunTime::deleteClient(this->getSocketFd());
    }
}

void Client::concatenateRequestData(std::string data) {
    if (this->_state == COMPLETE) {
        return;
    }

    this->_rawRequest.append(data);

    if (this->_state == READING_HEADER && this->_rawRequest.find("\r\n\r\n") != std::string::npos) {
        this->request.parseRequestLine(this->_rawRequest);
        this->request.parseHeaders(this->_rawRequest);

        if (!this->_serverListen.getServerBlock().isUriValid(this->request.getUri())) {
            this->response.setErrorPage(404);
            this->setState(COMPLETE);
            return;
        }
        if (
            !this->_serverListen.getServerBlock()
            .isLocationValid(
                this->request.getUri(),
                this->request.getMethod()
            )
        ) {
            this->response.setErrorPage(405);
            this->setState(COMPLETE);
            return;
        }
        this->setState(READING_BODY);
    }

    if (this->_state == READING_BODY) {
        std::string contentLengthStr = this->request.getHeaderValue("Content-Length");
        if (!contentLengthStr.empty()) {
            int contentLength = std::atoi(contentLengthStr.c_str());
            size_t bodyStartPos = this->_rawRequest.find("\r\n\r\n") + 4;
            size_t bodyLength = this->_rawRequest.size() - bodyStartPos;
            
            if (bodyLength >= static_cast<size_t>(contentLength)) {
                this->request.parseBody(this->_rawRequest);
                this->setState(COMPLETE);
            }
        } else {
            this->setState(COMPLETE);
        }
    }
}

bool Client::isRequestComplete(void) {
    return (this->_state == COMPLETE);
}

int Client::getState(void) const {
    return (this->_state);
}

std::string &Client::getRawRequest(void) {
    return (this->_rawRequest);
}

HttpRequest &Client::getRequest(void) {
    return (this->request);
}

HttpResponse &Client::getResponse(void) {
    return (this->response);
}

void Client::setState(int state) {
    this->_state = state;
}
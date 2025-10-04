#include "../includes/WebservHeader.hpp"


Client::Client(void) {}

Client::Client(int clientFd) {
    std::cout << "Client got created..." << std::endl;
    this->_state = READING_HEADER;
    this->_clientFd = clientFd;
    this->_rawRequest = "";
    this->request = HttpRequest();
}

Client::Client(int state, int clientFd, HttpRequest request, HttpResponse response) {
    std::cout << "Client got created..." << std::endl;
    this->_state = state;
    this->_clientFd = clientFd;
    this->request = request;
    this->response = response;
}

Client::Client(const Client &src) {
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

int Client::getState(void) const {
    return (this->_state);
}

        // std::string &getRequest(void) const;
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

void Client::concatenateRequestData(std::string data) {
    if (this->_state == COMPLETE) {
        return;
    }

    this->_rawRequest.append(data);
    if (this->_state == READING_HEADER && this->_rawRequest.find("\r\n\r\n") != std::string::npos) {
        this->request.parseRequestLine(this->_rawRequest);
        this->request.parseHeaders(this->_rawRequest);
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
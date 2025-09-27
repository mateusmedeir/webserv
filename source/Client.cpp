#include "../includes/WebservHeader.hpp"


Client::Client(void) {}

Client::Client(int state, int clientFd, std::string request, std::string response) {
    std::cout << "Client got created..." << std::endl;
    this->_state = state;
    this->_clientFd = clientFd;
    this->_request = request;
    this->_response = response;
}

Client::Client(const Client &src) {
    *this = src;
}

Client &Client::operator=(const Client &src) {
    if (this != &src) {
        this->_request = src._request;
        this->_response = src._response;
        this->_state = src._state;
    }
    return (*this);
}

Client::~Client(void) {}

int Client::getState(void) const {
    return (this->_state);
}

        // std::string &getRequest(void) const;
std::string &Client::getRequest(void) {
    return (this->_request);
}

void Client::setState(int state) {
    this->_state = state;
}


void Client::concatenateClientRequest(std::string request) {
    std::cout << "Concatenating client request: " << request << std::endl;
    this->_request += request;
    if (this->_request.find("\r\n\r\n") != std::string::npos) {
        this->setState(COMPLETE);
    }
}

bool Client::isRequestComplete(void) {
    return (this->_state == COMPLETE);
}
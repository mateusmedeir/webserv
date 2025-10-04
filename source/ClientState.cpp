#include "../includes/WebservHeader.hpp"


ClientState::ClientState(void) {}

ClientState::ClientState(int state, int clientFd, std::string request, std::string response) {
    std::cout << "Client got created..." << std::endl;
    this->_state = state;
    this->_clientFd = clientFd;
    this->_request = request;
    this->_response = response;
}

ClientState::ClientState(const ClientState &src) {
    *this = src;
}

ClientState &ClientState::operator=(const ClientState &src) {
    if (this != &src) {
        this->_request = src._request;
        this->_response = src._response;
        this->_state = src._state;
    }
    return (*this);
}

ClientState::~ClientState(void) {}

int ClientState::getState(void) const {
    return (this->_state);
}

        // std::string &getRequest(void) const;
std::string &ClientState::getRequest(void) {
    return (this->_request);
}

void ClientState::setState(int state) {
    this->_state = state;
}

void ClientState::concatenateClientRequest(std::string request) {
    this->_request += request;
}

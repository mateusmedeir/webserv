#include "../includes/CgiPipeHandler.hpp"
#include "../includes/CgiHandler.hpp"
#include "../includes/RunTime.hpp"
#include "../includes/Client.hpp"

CgiPipeHandler::CgiPipeHandler(int pipeFd, int clientFd, bool isInputPipe)
    : EpollHandler(isInputPipe ? EPOLLOUT : EPOLLIN, pipeFd),
      _clientFd(clientFd), _isInputPipe(isInputPipe) {
}

CgiPipeHandler::~CgiPipeHandler() {
}

void CgiPipeHandler::handleEpollIn(void) {
    if (!_isInputPipe) {
        // É pipe de saída - ler dados do CGI
        try {
            Client* client = RunTime::getClient(_clientFd);
            if (client) {
                CgiHandler::handleCgiPipeOut(this->getSocketFd(), client);
            }
        } catch (...) {
            // Cliente não existe mais - limpar processo
            CgiHandler::cleanupClientProcess(_clientFd);
        }
    }
}

void CgiPipeHandler::handleEpollOut(void) {
    if (_isInputPipe) {
        // É pipe de entrada - escrever dados no CGI
        try {
            Client* client = RunTime::getClient(_clientFd);
            if (client) {
                CgiHandler::handleCgiPipeIn(this->getSocketFd(), client);
            }
        } catch (...) {
            // Cliente não existe mais - limpar processo
            CgiHandler::cleanupClientProcess(_clientFd);
        }
    }
}

int CgiPipeHandler::getClientFd() const {
    return _clientFd;
}

bool CgiPipeHandler::isInputPipe() const {
    return _isInputPipe;
}


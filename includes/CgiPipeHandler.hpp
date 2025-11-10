#pragma once

#include "WebservHeader.hpp"

class Client;

class CgiPipeHandler : public EpollHandler {
private:
    int _clientFd;          // FD do cliente relacionado
    bool _isInputPipe;      // true se é pipe de entrada, false se é pipe de saída
    
public:
    CgiPipeHandler(int pipeFd, int clientFd, bool isInputPipe);
    virtual ~CgiPipeHandler();
    
    virtual void handleEpollIn(void);
    virtual void handleEpollOut(void);
    
    int getClientFd() const;
    bool isInputPipe() const;
};


#pragma once

# include "WebservHeader.hpp"

class ServerListen;

class Client {
    private:
        int             _state;
        int             _clientFd;
        std::string     _rawRequest;
        ServerListen    _serverListen;
    public:
        HttpRequest     request;
        HttpResponse    response;

        Client(void);
        Client(int clientFd, ServerListen &serverListen);
        Client(int state, int clientFd, HttpRequest request, HttpResponse response);
        Client(const Client &src);
        Client &operator=(const Client &src);
        ~Client(void);

        void concatenateRequestData(std::string data);        
        bool isRequestComplete(void);
        
        int getState(void) const;
        std::string &getRawRequest(void);
        HttpRequest &getRequest(void);
        HttpResponse &getResponse(void);

        void setState(int state);
};
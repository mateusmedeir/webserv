#pragma once

# include "WebservHeader.hpp"

class ServerListen;
class Cgi;

class Client : public EpollHandler {
    private:
        int             _state;
        std::string     _rawRequest;
        ServerListen    &_serverListen;
        Cgi             *_cgi;  // Ponteiro para CGI (se houver)
        time_t          _lastActivity;  // Timestamp da última atividade
    public:
        HttpRequest     request;
        HttpResponse    response;

        Client(int clientFd, ServerListen &serverListen);
        Client(const Client &src);
        Client &operator=(const Client &src);
        ~Client(void);

        virtual void handleEpollIn(void);
        void concatenateRequestData(std::string data);
        bool isRequestComplete(void);
        
        int getState(void) const;
        std::string &getRawRequest(void);
        HttpRequest &getRequest(void);
        HttpResponse &getResponse(void);

        void setState(int state);
        
        // CGI management
        void startCgiExecution(const std::string &scriptPath);
        void checkCgiCompletion(void);
        bool hasCgi(void) const;
        Cgi* getCgi(void) const;
        
        // Timeout management
        bool isTimedOut(int timeoutSeconds) const;
        void updateActivity(void);
};
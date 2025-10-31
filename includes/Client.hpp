#pragma once

# include "WebservHeader.hpp"

class ServerListen;

class Client : public EpollHandler {
    private:
        int             _state;
        std::string     _rawRequest;
        ServerListen    &_serverListen;
        time_t          _lastActivity;  // Timestamp da última atividade
        std::string     _pendingResponse;  // Resposta pendente de envio (caso send() parcial)
        size_t          _responseOffset;   // Offset atual da resposta sendo enviada
    public:
        HttpRequest     request;
        HttpResponse    response;

        Client(int clientFd, ServerListen &serverListen);
        Client(const Client &src);
        Client &operator=(const Client &src);
        ~Client(void);

        virtual void handleEpollIn(void);
        virtual void handleEpollOut(void);  // Para envio assíncrono quando socket está pronto
        void concatenateRequestData(std::string data);
        bool isRequestComplete(void);
        bool sendResponse(const std::string &responseStr);  // Envia resposta com tratamento correto de erros
        
        int getState(void) const;
        std::string &getRawRequest(void);
        HttpRequest &getRequest(void);
        HttpResponse &getResponse(void);

        void setState(int state);
        
        // Timeout management
        bool isTimedOut(int timeoutSeconds) const;
        void updateActivity(void);
};
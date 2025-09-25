#pragma once

# include "WebservHeader.hpp"
# include <string>

class ClientState {
    private:
        int         _state;
        int         _clientFd;
        std::string _request;
        std::string _response;
    public:
        ClientState(void);
        ClientState(int state, int clientFd, std::string request, std::string response);
        ClientState(const ClientState &src);
        ClientState &operator=(const ClientState &src);

        ~ClientState(void);

        int getState(void) const;
        std::string &getRequest(void);

        void setState(int state);

        void concatenateClientRequest(std::string request);
};
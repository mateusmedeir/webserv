#pragma once

# include "WebservHeader.hpp"
# include <string>

class Client {
    private:
        int         _state;
        int         _clientFd;
        std::string _request;
        std::string _response;
    public:
        Client(void);
        Client(int state, int clientFd, std::string request, std::string response);
        Client(const Client &src);
        Client &operator=(const Client &src);

        ~Client(void);

        int getState(void) const;
        std::string &getRequest(void);

        void setState(int state);

        void concatenateClientRequest(std::string request);

        bool isRequestComplete(void);
};
#pragma once

# include "WebservHeader.hpp"

class NewEpollInstance;

class NewRunTime : public NewEpollInstance {
    private:
        int                 _serverFd;
        struct sockaddr_in  _serverAddr;
    public:
        NewRunTime(void);
        NewRunTime(const NewRunTime &src);
        NewRunTime &operator=(const NewRunTime &src);

        ~NewRunTime(void);

        virtual void manipInterestList(int operation, uint32_t events, int socketFd);

        int getServerFd(void) const;
};
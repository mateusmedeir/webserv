#pragma once

# include <netinet/in.h>

class RunTime {
    private:
        int     serverFd;
        struct  sockaddr_in addr;
    public:
        RunTime(void);
        ~RunTime(void);
        RunTime(const RunTime &src);
        RunTime &operator=(const RunTime &src);
};
#include "RunTime.hpp"

        // RunTime(void);
RunTime::RunTime(void) {}

        // ~RunTime(void);
RunTime::~RunTime(void) {}

        // RunTime(const RunTime &src);
RunTime::RunTime(const RunTime &src) {
    *this = src;
}
        // RunTime &operator=(const RunTime &src);
RunTime &RunTime::operator=(const RunTime &src) {
    if (this != &src) {
        this->serverFd = src.serverFd;
        this->addr = src.addr;
    }
    return (*this);
}
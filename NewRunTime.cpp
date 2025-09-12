#include "WebservHeader.hpp"
#include "NewRunTime.hpp"

// NewRunTime(void);
NewRunTime::NewRunTime(void) {
    std::cout << "New Run time got created." << std::endl;
    this->_epollInstance = NewEpollInstance();
}

// NewRunTime(const NewRunTime &src);
NewRunTime::NewRunTime(const NewRunTime &src) {
    *this = src;
}

// NewRunTime &operator=(const NewRunTime &src);
NewRunTime &NewRunTime::operator=(const NewRunTime &src) {
    if (this != &src) {
        this->_serverFd = src._serverFd;
        this->_serverAddr = src._serverAddr;
        this->_epollInstance = src._epollInstance;
    }
    return (*this);
}

// ~NewRunTime(void);
NewRunTime::~NewRunTime(void) {

}
#include "StdLogHandler.hpp"

// StdLogHandler();
StdLogHandler::StdLogHandler(): LogHandler() {}

// ~StdLogHandler();
StdLogHandler::~StdLogHandler() {}

// virtual void handleDebug(t_logEvent event);
void StdLogHandler::handleDebug(t_logEvent event) {
    std::time_t timestamp = std::time(NULL);
    std::string result = std::ctime(&timestamp);
    int pos = result.find("\n");

    if (pos == std::string::npos) {
        std::cout << "[DEBUG] " << event.message << std::endl;
    }
    else {
        // Usado para retirar o `\n` do final da string result
        std::string aux = result.erase(pos, 1);
        std::cout << aux << " [DEBUG] " << event.message << std::endl;
    }

    // char buffer[32];
    // // Www Mmm dd hh:mm:ss yyyy\n == 25 + 1 do \0
    // std::strncpy(buffer, std::ctime(&timestamp), 26);
    // std::cout << buffer << " [DEBUG] " << event.message << std::endl;
}

// virtual void handleError(t_logEvent event);
void StdLogHandler::handleError(t_logEvent event) {
    std::time_t timestamp = std::time(NULL);
    std::string result = std::ctime(&timestamp);
    int pos = result.find("\n");

    if (pos == std::string::npos) {
        std::cout << "[ERROR] " << event.message << std::endl;
    }
    else {
        // Usado para retirar o `\n` do final da string result
        std::string aux = result.erase(pos, 1);
        std::cout << aux << " [ERROR] " << event.message << std::endl;
    }
    // std::time_t timestamp = std::time(NULL);
    // std::cout << std::ctime(&timestamp) << " [ERROR] " << event.message << std::endl;
}

// virtual void handleInfo(t_logEvent event);
void StdLogHandler::handleInfo(t_logEvent event) {
    std::time_t timestamp = std::time(NULL);
    std::string result = std::ctime(&timestamp);
    int pos = result.find("\n");

    if (pos == std::string::npos) {
        std::cout << "[INFO] " << event.message << std::endl;
    }
    else {
        // Usado para retirar o `\n` do final da string result
        std::string aux = result.erase(pos, 1);
        std::cout << aux << " [INFO] " << event.message << std::endl;
    }
    // std::time_t timestamp = std::time(NULL);
    // std::cout << std::ctime(&timestamp) << " [INFO] " << event.message << std::endl;
}

// virtual void handleWarning(t_logEvent event);
void StdLogHandler::handleWarning(t_logEvent event) {
    std::time_t timestamp = std::time(NULL);
    std::string result = std::ctime(&timestamp);
    int pos = result.find("\n");

    if (pos == std::string::npos) {
        std::cout << "[WARNING] " << event.message << std::endl;
    }
    else {
        // Usado para retirar o `\n` do final da string result
        std::string aux = result.erase(pos, 1);
        std::cout << aux << " [WARNING] " << event.message << std::endl;
    }
    // std::time_t timestamp = std::time(NULL);
    // std::cout << std::ctime(&timestamp) << " [WARNING] " << event.message << std::endl;
}
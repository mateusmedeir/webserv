#include "FileLogHandler.hpp"

// FileLogHandler(void);
FileLogHandler::FileLogHandler(std::string filename): LogHandler(), _logFile(filename, std::ofstream::out | std::ofstream::app) {}

// ~FileLogHandler(void);
FileLogHandler::~FileLogHandler(void) {_logFile.close();}

// virtual void handleDebug(t_logEvent event);
void FileLogHandler::handleDebug(t_logEvent event) {
    std::time_t timestamp = std::time(NULL);
    std::string result = std::ctime(&timestamp);
    int pos = result.find("\n");

    if (pos == std::string::npos) {
        _logFile << "[DEBUG] " << event.message << std::endl;
    }
    else {
        // Usado para retirar o `\n` do final da string result
        std::string aux = result.erase(pos, 1);
        _logFile << aux << " [DEBUG] " << event.message << std::endl;
    }
}

// virtual void handleError(t_logEvent event);
void FileLogHandler::handleError(t_logEvent event) {
    std::time_t timestamp = std::time(NULL);
    std::string result = std::ctime(&timestamp);
    int pos = result.find("\n");

    if (pos == std::string::npos) {
        _logFile << "[ERROR] " << event.message << std::endl;
    }
    else {
        // Usado para retirar o `\n` do final da string result
        std::string aux = result.erase(pos, 1);
        _logFile << aux << " [ERROR] " << event.message << std::endl;
    }
}

// virtual void handleInfo(t_logEvent event);
void FileLogHandler::handleInfo(t_logEvent event) {
    std::time_t timestamp = std::time(NULL);
    std::string result = std::ctime(&timestamp);
    int pos = result.find("\n");

    if (pos == std::string::npos) {
        _logFile << "[INFO] " << event.message << std::endl;
    }
    else {
        // Usado para retirar o `\n` do final da string result
        std::string aux = result.erase(pos, 1);
        _logFile << aux << " [INFO] " << event.message << std::endl;
    }
}

// virtual void handleWarning(t_logEvent event);
void FileLogHandler::handleWarning(t_logEvent event) {
    std::time_t timestamp = std::time(NULL);
    std::string result = std::ctime(&timestamp);
    int pos = result.find("\n");

    if (pos == std::string::npos) {
        _logFile << "[WARNING] " << event.message << std::endl;
    }
    else {
        // Usado para retirar o `\n` do final da string result
        std::string aux = result.erase(pos, 1);
        _logFile << aux << " [WARNING] " << event.message << std::endl;
    }
}
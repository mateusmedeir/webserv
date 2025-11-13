#include "Logger.hpp"

Logger *Logger::_instance = NULL;

Logger::Logger(void) {}

Logger::Logger(enum LogLevel level, LogHandler *handler): _level(level), _handler(handler) {}

int Logger::initLogger(enum LogLevel level, LogHandler *handler) {
    if (Logger::_instance == NULL) {
        Logger::_instance = new Logger(level, handler);
        return (0);
    }
    return (-1);
}

void Logger::debug(std::string message){
    if (Logger::_instance == NULL || Logger::_instance->_handler == NULL || Logger::_instance->_level > DEBUG) {
        return ;
    }
    t_logEvent event = {.level = DEBUG, .message = message};
    Logger::_instance->_handler->handleDebug(event);
}

void Logger::error(std::string message){
    if (Logger::_instance == NULL || Logger::_instance->_handler == NULL || Logger::_instance->_level > ERROR) {
        return ;
    }
    t_logEvent event = {.level = ERROR, .message = message};
    Logger::_instance->_handler->handleError(event);
}

void Logger::info(std::string message){
    if (Logger::_instance == NULL || Logger::_instance->_handler == NULL || Logger::_instance->_level > INFO) {
        return ;
    }
    t_logEvent event = {.level = INFO, .message = message};
    Logger::_instance->_handler->handleInfo(event);
}

void Logger::warning(std::string message){
    if (Logger::_instance == NULL || Logger::_instance->_handler == NULL || Logger::_instance->_level > WARNING) {
        return ;
    }
    t_logEvent event = {.level = WARNING, .message = message};
    Logger::_instance->_handler->handleWarning(event);
}
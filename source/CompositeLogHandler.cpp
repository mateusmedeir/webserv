#include "../includes/WebservHeader.hpp"

CompositeLogHandler::CompositeLogHandler(void): LogHandler() {}

CompositeLogHandler::~CompositeLogHandler(void) {}

void CompositeLogHandler::handleDebug(t_logEvent event) {
    for (size_t i = 0; i < _handlers.size(); i++)
		_handlers[i]->handleDebug(event);
}

void CompositeLogHandler::handleError(t_logEvent event) {
    for (size_t i = 0; i < _handlers.size(); i++)
		_handlers[i]->handleError(event);   
}

void CompositeLogHandler::handleInfo(t_logEvent event) {
    for (size_t i = 0; i < _handlers.size(); i++)
		_handlers[i]->handleInfo(event);
}

void CompositeLogHandler::handleWarning(t_logEvent event) {
    for (size_t i = 0; i < _handlers.size(); i++)
		_handlers[i]->handleWarning(event);
}

void CompositeLogHandler::addHandler(LogHandler *newHandler) {
    this->_handlers.push_back(newHandler);
}
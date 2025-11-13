#pragma once

#include "Logger.hpp"
#include <ctime>

class StdLogHandler : public LogHandler {
    public:
        StdLogHandler();
        ~StdLogHandler();
        virtual void handleDebug(t_logEvent event);
        virtual void handleError(t_logEvent event);
        virtual void handleInfo(t_logEvent event);
        virtual void handleWarning(t_logEvent event);
};
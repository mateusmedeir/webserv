#pragma once

# include "WebservHeader.hpp"

class StdLogHandler : public LogHandler {
    public:
        StdLogHandler();
        ~StdLogHandler();
        virtual void handleDebug(t_logEvent event);
        virtual void handleError(t_logEvent event);
        virtual void handleInfo(t_logEvent event);
        virtual void handleWarning(t_logEvent event);
};
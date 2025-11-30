#pragma once

# include "WebservHeader.hpp"

class CompositeLogHandler : public LogHandler {
    private:
        std::vector<LogHandler *> _handlers; 
    public:
        CompositeLogHandler(void);
        virtual ~CompositeLogHandler(void);

        void    addHandler(LogHandler *newHandler);

        virtual void handleDebug(t_logEvent event);
        virtual void handleError(t_logEvent event);
        virtual void handleInfo(t_logEvent event);
        virtual void handleWarning(t_logEvent event);
};
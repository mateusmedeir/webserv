#pragma once

# include "WebservHeader.hpp"

class FileLogHandler : public LogHandler {
    private:
        std::ofstream _logFile;

    public:
        FileLogHandler(std::string filename);
        virtual ~FileLogHandler(void);
        virtual void handleDebug(t_logEvent event);
        virtual void handleError(t_logEvent event);
        virtual void handleInfo(t_logEvent event);
        virtual void handleWarning(t_logEvent event);
};
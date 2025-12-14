#pragma once

# include "WebservHeader.hpp"

class LogHandler {
    public:
        // LogHandler() = default;
        // virtual ~LogHandler() = default;
        LogHandler();
        virtual ~LogHandler();
        virtual void handleDebug(t_logEvent event) = 0;
        virtual void handleError(t_logEvent event) = 0;
        virtual void handleInfo(t_logEvent event) = 0;
        virtual void handleWarning(t_logEvent event) = 0;
};

class Logger {
    private:
        static Logger   *_instance; // Ptr estático para armazenar a ref. da única instancia existente
        enum LogLevel   _level;
        LogHandler      *_handler;

        Logger(void); // Construtor privado para não conseguirmos instanciar um novo Logger
        // Logger(const Logger &src) = delete; // Construtor de cópia deletado. Previnindo nova instancia
        // Logger &operator=(const Logger &src) = delete; // Operador de cópia deletado. Previnindo cópia da instancia
        Logger(const Logger &src); // Construtor de cópia deletado. Previnindo nova instancia
        Logger &operator=(const Logger &src); // Operador de cópia deletado. Previnindo cópia da instancia

        Logger(enum LogLevel level, LogHandler *handler);
    public:
        ~Logger(void);
        static int      initLogger(enum LogLevel level, LogHandler *handler);
        static void     deleteInstance(void);
        static void     debug(std::string message);
        static void     error(std::string message);
        static void     info(std::string message);
        static void     warning(std::string message);
};

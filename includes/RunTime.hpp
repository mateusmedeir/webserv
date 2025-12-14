#pragma once

# include "WebservHeader.hpp"

class RunTime {
    private:
        static RunTime              *_instance;
        ConfigFile                  _config;

        RunTime(void);
        RunTime(int ac, char **av);
        RunTime(const RunTime &src);
        RunTime &operator=(const RunTime &src);

    public:
        ~RunTime(void);

        static void initializeRuntime(int ac, char **av);
        static void deleteInstance(void);

        static void initServerSockets(void);
        
        static RunTime &getInstance(void);
        static ConfigFile &getConfig(void);
};
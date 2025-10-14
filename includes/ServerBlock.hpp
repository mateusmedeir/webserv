#pragma once

# include "WebservHeader.hpp"

class ConfigFile;
class LocationBlock;

typedef struct s_listen
{
    unsigned int    host;
    int             port;
} t_listen;

class ServerBlock
{
    private:
        ConfigFile&                             _config;
        std::vector<std::string>                _serverNames;
        std::vector<t_listen>                   _listen;
        std::pair<bool, size_t>                 _maxBodySize;
        std::pair<bool, std::string>            _root;
        std::map<std::string, LocationBlock>    _locations;
        std::map<int, std::string>              _errorPages;

    public:
        ServerBlock(ConfigFile &config);
        ~ServerBlock();

        ServerBlock &operator=(const ServerBlock &src);
        bool operator==(const ServerBlock &other) const;
        
        void printServerBlock();

        void addListens();
        void addServerNames();
        void addMaxBodySize();
        void addRoot();
        void addErrorPages();
        void addLocation();

        //| Getters
        std::vector<std::string> getServerNames() const;
        std::vector<t_listen> getListen() const;
        std::pair<bool, size_t> getMaxBodySize() const;
        std::pair<bool, std::string> getRoot() const;
        std::map<int, std::string> getErrorPages() const;
        std::map<std::string, LocationBlock> getLocations() const;
};
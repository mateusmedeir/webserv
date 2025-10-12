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
        std::vector<std::string>                _serverNames;
        std::vector<t_listen>                   _listen;
        std::pair<bool, size_t>                 _maxBodySize;
        std::pair<bool, std::string>            _root;
        std::map<std::string, LocationBlock>    _locations;
        std::map<int, std::string>              _errorPages;

    public:
        ServerBlock();
        ServerBlock(ConfigFile &config);
        ~ServerBlock();

        bool operator==(const ServerBlock &other) const;
        
        void printServerBlock();

        void addListens(ConfigFile &config);
        void addServerNames(ConfigFile &config);
        void addMaxBodySize(ConfigFile &config);
        void addRoot(ConfigFile &config);
        void addErrorPages(ConfigFile &config);
        void addLocation(ConfigFile &config);

        //| Getters
        std::vector<std::string> getServerNames() const;
        std::vector<t_listen> getListen() const;
        std::pair<bool, size_t> getMaxBodySize() const;
        std::pair<bool, std::string> getRoot() const;
        std::map<int, std::string> getErrorPages() const;
        std::map<std::string, LocationBlock> getLocations() const;
};
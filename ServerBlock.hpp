#pragma once

#include <vector>               //| std::vector
#include <map>                  //| std::map
#include <string>               //| std::string
#include <utility>              //| std::pair
#include <cstddef>              //| std::size_t
#include <cstdlib>              //| std::atoi
#include "ParserConfigFile.hpp" //| ParserConfigFile
#include "LocationBlock.hpp"    //| LocationBlock

#include <iostream> //| Para testes, remover depois


class LocationBlock; //| Para poder usar LocationBlock em ServerBlock

typedef struct s_listen
{
    unsigned int    host;
    int             port;
} t_listen;

class ServerBlock
{
    public:
        ServerBlock(std::vector<std::string> &tokens);
        ~ServerBlock();

        //| Getters
        std::vector<std::string> getServerNames() const;
        std::vector<t_listen> getListen() const;
        std::pair<bool, size_t> getMaxBodySize() const;
        std::pair<bool, std::string> getRoot() const;
        std::map<int, std::string> getErrorPages() const;
        std::map<std::string, LocationBlock> getLocations() const;

    private:
		std::vector<std::string>                _serverNames;
		std::vector<t_listen>                   _listen;
		std::pair<bool, size_t>                 _maxBodySize;
        std::pair<bool, std::string>            _root;
        std::map<std::string, LocationBlock>    _locations;
        std::map<int, std::string>              _errorPages;

        void printServerBlock();

        void addListens(std::vector<std::string> &tokens);
        void addServerNames(std::vector<std::string> &tokens);
        void addMaxBodySize(std::vector<std::string> &tokens);
        void addRoot(std::vector<std::string> &tokens);
        void addErrorPages(std::vector<std::string> &tokens);
        void addLocation(std::vector<std::string> &tokens);
};
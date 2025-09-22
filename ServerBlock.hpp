#pragma once

#include <vector>               //| std::vector
#include <string>               //| std::string
#include <utility>              //| std::pair
#include <cstddef>              //| std::size_t
#include <cstdlib>              //| std::atoi
#include "ParserConfigFile.hpp" //| ParserConfigFile

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

    private:
		std::vector<std::string>	_serverNames;
		std::vector<t_listen>		_listen;
		std::pair<bool, size_t>		_maxBodySize;

        //| Setters
		//void setListen(std::vector<std::string> &tokens);      //| Fazer
		//void setServerName(std::vector<std::string> &tokens);  //| Fazer
		void setMaxBodySize(std::vector<std::string> &tokens);
		//void setErrorPage(std::vector<std::string> &tokens);   //| Fazer
		//void setLocation(std::vector<std::string> &tokens);    //| Fazer
		//void setRoot(std::vector<std::string> &tokens);        //| Fazer
};
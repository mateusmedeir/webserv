#pragma once

#include <string>    //| std::string
#include <fstream>   //| std::ifstream
#include <vector>    //| std::vector
#include <sstream>   //| std::istringstream
#include <cctype>    //| std::isspace

class ParserConfigFile
{
	public:
		static void readFile(const std::string &filename, std::string &content);
		static void trim(std::string &content);
		static void removeComments(std::string &content);
		static void cleanFile(const std::string &filename, std::string &content);
		static std::vector<std::string> tokenizeContent(const std::string &content);
		static void parser(const std::string &filename, std::vector<std::string> &tokens);
	
	private:
		typedef struct s_listen
		{
			unsigned int host;
			int port;
		} t_listen;

		std::vector<std::string> _serverName;
		std::vector<t_listen> _listen;
		size_t _maxBodySize;

		void setServerName(std::vector<std::string> &tokens);  //| Fazer
		void setListen(std::vector<std::string> &tokens);      //| Fazer
		void setRoot(std::vector<std::string> &tokens);        //| Fazer
		void setMaxBodySize(std::vector<std::string> &tokens); //| Fazer
		void setErrorPage(std::vector<std::string> &tokens);   //| Fazer
		void setLocation(std::vector<std::string> &tokens);    //| Fazer
};
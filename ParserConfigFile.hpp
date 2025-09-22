#pragma once

#include <string>          //| std::string
#include <fstream>         //| std::ifstream
#include <vector>          //| std::vector
#include <sstream>         //| std::istringstream
#include <cctype>          //| std::isspace
#include "ServerBlock.hpp" //| ServerBlock

class ParserConfigFile
{
	public:
		static void readFile(const std::string &filename, std::string &content);
		static void trim(std::string &content);
		static void removeComments(std::string &content);
		static void cleanFile(const std::string &filename, std::string &content);
		static std::vector<std::string> tokenizeContent(const std::string &content);
		static void parser(const std::string &filename, std::vector<std::string> &tokens);
		static void removeTokens(std::vector<std::string> &tokens, size_t amount);
};
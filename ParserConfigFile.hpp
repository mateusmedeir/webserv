#pragma once

#include <string>          //| std::string
#include <fstream>         //| std::ifstream
#include <vector>          //| std::vector
#include <sstream>         //| std::istringstream
#include <cctype>          //| std::isspace
#include "ServerBlock.hpp" //| ServerBlock


enum TypeValidation
{
	EMPTY = 0,          //| Verifica se tokens está vazio
	SEMICOLON = 1,      //| Verifica se está vazio OU é ponto e vírgula
	DIFF_SEMICOLON = 2, //| Verifica se está vazio OU não é ponto e vírgula
	END_OF_FILE = 3     //| Verifica se é o final do arquivo
};

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
		static void verifyToken(const std::vector<std::string> &tokens, TypeValidation type, const std::string &message);
};
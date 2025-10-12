#pragma once

# include "WebservHeader.hpp"

enum TypeValidation
{
	EMPTY = 0,          //| Verifica se tokens está vazio
	SEMICOLON = 1,      //| Verifica se está vazio OU é ponto e vírgula
	DIFF_SEMICOLON = 2, //| Verifica se está vazio OU não é ponto e vírgula
	END_OF_FILE = 3     //| Verifica se é o final do arquivo
};

class ServerBlock;
class ServerListen;

class ConfigFile {
	private:
		std::vector<std::string>    _tokens;
		std::vector<ServerBlock>    _serverBlocks;
		std::vector<ServerListen>   _serverListens;
	public:
		ConfigFile(void);
		~ConfigFile(void);

		std::vector<std::string> tokenizeContent(const std::string &content);
		void parser(const std::string &filename);
		void removeTokens(size_t amount);
		void verifyToken(TypeValidation type, const std::string &message);

		static void readFile(const std::string &filename, std::string &content);
		static void trim(std::string &content);
		static void removeComments(std::string &content);
		static void cleanFile(const std::string &filename, std::string &content);

		std::vector<std::string> getTokens(void);
		std::vector<ServerBlock> getServerBlocks(void) const;
		std::vector<ServerListen> getServerListens(void) const;
};
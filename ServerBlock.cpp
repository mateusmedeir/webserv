#include "ServerBlock.hpp"

ServerBlock::ServerBlock(std::vector<std::string> &tokens): _maxBodySize(false, 0)
{
	ParserConfigFile::removeTokens(tokens, 2); //| Remove os 2 primeiros tokens ('server' e '{')

	if (tokens.size() == 0)
		throw std::runtime_error("Configuração inválida: não foi encontrado nenhum servidor");

	while (tokens.size() > 0)
	{
		//if (tokens[0] == "listen")
			//setListen(tokens); //| Fazer
		//else if (tokens[0] == "server_name")
			//setServerName(tokens); //| Fazer
		if (tokens[0] == "client_max_body_size")
			setMaxBodySize(tokens);
		//else if (tokens[0] == "error_page")
			//setErrorPage(tokens);
		//else if (tokens[0] == "location")
			//setLocation(tokens); //| Fazer
		//else if (tokens[0] == "root")
			//setRoot(tokens); //| Fazer
		else if (tokens[0] == "}")
			break;
		else
			throw std::runtime_error("Configuração inválida: token inválido");
	}
}

ServerBlock::~ServerBlock() {}

std::vector<std::string> ServerBlock::getServerNames() const { return this->_serverNames; }
std::vector<t_listen> ServerBlock::getListen() const { return this->_listen; }
std::pair<bool, size_t> ServerBlock::getMaxBodySize() const { return this->_maxBodySize; }

void ServerBlock::setMaxBodySize(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1);

    if (this->_maxBodySize.first == true) //| Verifica se o client_max_body_size já definido
        throw std::runtime_error("Configuração inválida: client_max_body_size já foi definido");
    this->_maxBodySize.first = true;

    std::string value = tokens[0];
    size_t i = 0;
    while (i < value.size() - 1)
    {
        if (!isdigit(value[i]))
            throw std::runtime_error("Configuração inválida: client_max_body_size deve ser um número");
        i++;
    }

    this->_maxBodySize.second = std::atoi(value.c_str());
    if (value[i] == 'G')
        this->_maxBodySize.second *= 1024 * 1024 * 1024;
    else if (value[i] == 'M')
        this->_maxBodySize.second *= 1024 * 1024;
    else if (value[i] == 'K')
        this->_maxBodySize.second *= 1024;
    else if (value[i] == 'B')
        this->_maxBodySize.second *= 1;
    else
        throw std::runtime_error("Configuração inválida: client_max_body_size deve ser um número seguido de unidade");

    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o argumento de max_body_size
    if (tokens.size() == 0 || tokens[0] != ";")
        throw std::runtime_error("Configuração inválida: esperava um ponto e vírgula");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}
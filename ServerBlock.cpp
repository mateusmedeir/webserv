#include "ServerBlock.hpp"


ServerBlock::ServerBlock(std::vector<std::string> &tokens): _maxBodySize(false, 0), _root(false, "./")
{
	ParserConfigFile::removeTokens(tokens, 2); //| Remove os 2 primeiros tokens ('server' e '{')

	if (tokens.size() == 0)
		throw std::runtime_error("Configuração inválida: não foi encontrado nenhum servidor");

	while (tokens.size() > 0)
	{
		if (tokens[0] == "listen")
			addListens(tokens);
		else if (tokens[0] == "server_name")
			addServerNames(tokens);
		else if (tokens[0] == "client_max_body_size")
			addMaxBodySize(tokens);
		//else if (tokens[0] == "error_page")
			//addErrorPage(tokens);
		//else if (tokens[0] == "location") //| Vai ser um classe LocationBlock
			//addLocation(tokens); //| Fazer
		else if (tokens[0] == "root")
			addRoot(tokens);
		else if (tokens[0] == "}")
        {
            ParserConfigFile::removeTokens(tokens, 1);
            break;
        }
		else
			throw std::runtime_error("Configuração inválida: token inválido");
	}

    //| Fazer verificação para ver se os atributos estão corretos.
    if (this->_maxBodySize.second == 0)
        throw std::runtime_error("Configuração inválida: client_max_body_size não pode ser zero.");

    printServerBlock();
}

ServerBlock::~ServerBlock() {}

std::vector<std::string> ServerBlock::getServerNames() const { return this->_serverNames; }
std::vector<t_listen> ServerBlock::getListen() const { return this->_listen; }
std::pair<bool, size_t> ServerBlock::getMaxBodySize() const { return this->_maxBodySize; }

void ServerBlock::printServerBlock()
{
    std::cout << "Server names: " << std::endl;
	for (std::vector<std::string>::iterator it = this->_serverNames.begin(); it != this->_serverNames.end(); it++)
		std::cout << *it << " " << std::endl;

	std::cout << "Max body size: " << this->_maxBodySize.second << std::endl;

    std::cout << "Root: " << this->_root.second << std::endl;

    std::cout << "Listens: " << std::endl;
    for (size_t i = 0; i < this->_listen.size(); i++)
        std::cout << "Host[" << i << "]: " << this->_listen[i].host << " Port[" << i << "]: " << this->_listen[i].port << std::endl;
}

static unsigned int strToIpv4(std::string s)
{
    if (s == "localhost")
        s = "127.0.0.1";

    std::istringstream ss(s);
    std::string octet_str;
    std::vector<unsigned int> octets;

    while (std::getline(ss, octet_str, '.'))
    {
        unsigned int octet;
        if (!(std::istringstream(octet_str) >> octet) || octet > 255) //| Verificando se é só número e se o octeto é maior que 225
        {
            throw std::runtime_error("Host inválido, octeto > 255 ou caracteres não numéricos");
            return (0);
        }
        octets.push_back(octet);
    }

    if (octets.size() != 4) //| Verificando se tem mais de 4 octetos
    {
        throw std::runtime_error("Host inválido, mais de 4 octetos");
        return (0);
    }

    return ((octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) | octets[3]);
}

void ServerBlock::addListens(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1);
    if (tokens.size() == 0)
        throw std::runtime_error("Configuração inválida: não foi encontrado nenhum listen");

    std::string host_port = tokens[0];
    if (host_port == ";") //| Caso não seja especificado nenhum Host e Port, tem a padrão 0.0.0.0:80
        host_port = "0.0.0.0:80";

    //| Pegar conteúdo que vem antes do : e transformar em Host
    std::string before = host_port.substr(0, host_port.find(':'));
    unsigned int host = strToIpv4(before);

    //| Pegar conteúdo que vem depois do : e transformar em Port
    //| Para o Port, tem que verificar antes se tem o : se não vai duplicar o Host > listen localhost -> Host: localhost & Port: localhost
    std::string after = host_port.substr(host_port.find(':') + 1, host_port.length() - host_port.find(':'));
    int port;
    if (after == before) //| Caso a porta não seja especificada
        port = 80;
    else
    {
        size_t i = 0;
        while (i < after.size() - 1)
        {
            if (!isdigit(after[i]))
                throw std::runtime_error("Configuração inválida: port deve ser um número");
            i++;
        }

        port = std::atoi(after.c_str());
        if (port < 1 || port > 65535)
            throw std::runtime_error("Configuração inválida: port deve ser um número entre 1 e 65535");
    }

    t_listen listen;
    listen.host = host;
    listen.port = port;

    //| Remover duplicatas de listen (?)
    for (size_t i = 0; i < this->_listen.size(); i++)
    {
        if (this->_listen[i].host == listen.host && this->_listen[i].port == listen.port)
            throw std::runtime_error("Duplicated listen"); //| Ou somente remover duplicatas?
    }

    this->_listen.push_back(listen);

    if (tokens[0] != ";")
        ParserConfigFile::removeTokens(tokens, 1); //| Removendo o argumento de listen
    if (tokens.size() == 0 || tokens[0] != ";")
        throw std::runtime_error("Configuração inválida: esperava um ponto e vírgula");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}

void ServerBlock::addServerNames(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1);
    if (tokens.size() == 0 || tokens[0] == ";")
        throw std::runtime_error("Configuração inválida: não foi encontrado nenhum server_name");

    std::vector<std::string> names;
    while (tokens[0] != ";") //| Enquanto não encontrar o ponto e vírgula, todos os argumentos devem ser nomes de servidor
    {
        names.push_back(tokens[0]);
        ParserConfigFile::removeTokens(tokens, 1);
    }

    for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); ++it)
        this->_serverNames.push_back(*it);        

    for (std::vector<std::string>::iterator it = this->_serverNames.begin(); it != this->_serverNames.end(); ++it) //| Removendo duplicatas
    {
        for (std::vector<std::string>::iterator jt = it + 1; jt != this->_serverNames.end(); )
        {
            if (*it == *jt)
                jt = this->_serverNames.erase(jt);
            else
                ++jt;
        }
    }

    if (tokens.size() == 0 || tokens[0] != ";")
        throw std::runtime_error("Configuração inválida: esperava um ponto e vírgula");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}

void ServerBlock::addMaxBodySize(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1);
    if (tokens.size() == 0 || tokens[0] == ";")
        throw std::runtime_error("Configuração inválida: não foi encontrado nenhum client_max_body_size");

    if (this->_maxBodySize.first == true) //| Verifica se o client_max_body_size já está definido
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

void ServerBlock::addRoot(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1);
    if (tokens.size() == 0 || tokens[0] == ";")
        throw std::runtime_error("Configuração inválida: não foi encontrado nenhuma root");

    if (this->_root.first == true) //| Verifica se o root já está definido
        throw std::runtime_error("Configuração inválida: root já foi definido");
    this->_root.first = true;

    this->_root.second = tokens[0];

    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o argumento de root
    if (tokens.size() == 0 || tokens[0] != ";")
        throw std::runtime_error("Configuração inválida: esperava um ponto e vírgula");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}
#include "ServerBlock.hpp"


ServerBlock::ServerBlock(std::vector<std::string> &tokens): _maxBodySize(false, 0), _root(false, "./")
{
	ParserConfigFile::removeTokens(tokens, 2); //| Remove os 2 primeiros tokens ('server' e '{')

	if (tokens.size() == 0)
		throw std::runtime_error("Configuração inválida: server: não foi encontrado nenhum servidor");

	while (tokens.size() > 0)
	{
		if (tokens[0] == "listen")
			addListens(tokens);
		else if (tokens[0] == "server_name")
			addServerNames(tokens);
		else if (tokens[0] == "client_max_body_size")
			addMaxBodySize(tokens);
		else if (tokens[0] == "error_page")
			addErrorPages(tokens);
		else if (tokens[0] == "location")
			addLocation(tokens);
		else if (tokens[0] == "root")
			addRoot(tokens);
		else if (tokens[0] == "}")
        {
            ParserConfigFile::removeTokens(tokens, 1);
            break;
        }
		else
			throw std::runtime_error("Configuração inválida: server: token inválido");
	}

    //| Fazer verificação para ver se os atributos estão corretos.
    if (this->_maxBodySize.second == 0)
        throw std::runtime_error("Configuração inválida: server: client_max_body_size não pode ser zero.");

    printServerBlock();
}

ServerBlock::~ServerBlock() {}

std::vector<std::string> ServerBlock::getServerNames() const { return this->_serverNames; }
std::vector<t_listen> ServerBlock::getListen() const { return this->_listen; }
std::pair<bool, size_t> ServerBlock::getMaxBodySize() const { return this->_maxBodySize; }
std::pair<bool, std::string> ServerBlock::getRoot() const { return this->_root; }
std::map<int, std::string> ServerBlock::getErrorPages() const { return this->_errorPages; }
std::map<std::string, LocationBlock> ServerBlock::getLocations() const { return this->_locations; }

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

    std::cout << "Error pages: " << std::endl;
    for (std::map<int, std::string>::iterator it = this->_errorPages.begin(); it != this->_errorPages.end(); ++it)
        std::cout << "Code: " << it->first << " | URI: " << it->second << std::endl;
}

static bool isAllNumber(std::string s)
{
    for (size_t i = 0; i < s.size(); i++)
        if (!isdigit(s[i]))
            return (false);
    return (true);
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
        if (octet_str.empty() || isAllNumber(octet_str) == false)
            throw std::runtime_error("Configuração inválida: listen: host: o octeto é inválido, contém caracteres não numéricos");

        unsigned int octet = std::atoi(octet_str.c_str());
        if (octet > 255)
            throw std::runtime_error("Configuração inválida: listen: host: o octeto é inválido, é maior que 255");

        octets.push_back(octet);
    }

    if (octets.size() != 4) //| Verificando se tem mais de 4 octetos
        throw std::runtime_error("Configuração inválida: listen: host: o host inválido, possui mais de 4 octetos");

    return ((octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) | octets[3]);
}

void ServerBlock::addListens(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o token 'listen'
    if (tokens.size() == 0)
        throw std::runtime_error("Configuração inválida: listen: não foi encontrado nenhum listen");

    std::string host_port = tokens[0];
    if (host_port == ";") //| Caso não seja especificado nenhum Host e Port, tem a padrão 0.0.0.0:80
        host_port = "0.0.0.0:80";

    //| Pegar conteúdo que vem antes do : e transformar em Host
    std::string before;
    if (host_port.find(':') != std::string::npos)
        before = host_port.substr(0, host_port.find(':'));
    else
        before = "0.0.0.0";
    unsigned int host = strToIpv4(before);

    //| Pegar conteúdo que vem depois do : e transformar em Port
    //| Para o Port, tem que verificar antes se tem o : se não vai duplicar o Host > listen localhost -> Host: localhost & Port: localhost
    std::string after = host_port.substr(host_port.find(':') + 1, host_port.length() - host_port.find(':'));
    int port;
    if (after == before) //| Caso a porta não seja especificada
        port = 80;
    else
    {
        if (isAllNumber(after) == false)
                throw std::runtime_error("Configuração inválida: listen: port: é inválido, deve ser um número");

        port = std::atoi(after.c_str());
        if (port < 1 || port > 65535)
            throw std::runtime_error("Configuração inválida: listen: port: é inválido, deve ser um número entre 1 e 65535");
    }

    t_listen listen;
    listen.host = host;
    listen.port = port;

    //| Remover duplicatas de listen (?)
    for (size_t i = 0; i < this->_listen.size(); i++)
        if (this->_listen[i].host == listen.host && this->_listen[i].port == listen.port)
            throw std::runtime_error("Configuração inválida: listen: listen duplicado"); //| Ou somente remover duplicatas?

    this->_listen.push_back(listen);

    if (tokens[0] != ";")
        ParserConfigFile::removeTokens(tokens, 1); //| Removendo o argumento de listen
    if (tokens.size() == 0 || tokens[0] != ";")
        throw std::runtime_error("Configuração inválida: listen: esperava um ponto e vírgula no final de listen");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}

void ServerBlock::addServerNames(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o token 'server_name'
    if (tokens.size() == 0 || tokens[0] == ";")
        throw std::runtime_error("Configuração inválida: server_name: não foi encontrado nenhum server_name");

    std::vector<std::string> names;
    while (tokens[0] != ";") //| Enquanto não encontrar o ponto e vírgula, todos os argumentos devem ser nomes de servidor
    {
        if (tokens[0] == tokens.back())
            throw std::runtime_error("Configuração inválida: server_name: final do arquivo encontrado");
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
        throw std::runtime_error("Configuração inválida: server_name: esperava um ponto e vírgula no final de server_name");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}

void ServerBlock::addMaxBodySize(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o token 'client_max_body_size'
    if (tokens.size() == 0 || tokens[0] == ";")
        throw std::runtime_error("Configuração inválida: client_max_body_size: não foi encontrado nenhum client_max_body_size");

    if (this->_maxBodySize.first == true) //| Verifica se o client_max_body_size já está definido
        throw std::runtime_error("Configuração inválida: client_max_body_size: client_max_body_size já foi definido");
    this->_maxBodySize.first = true;

    std::string value = tokens[0];
    size_t i = 0;
    while (i < value.size() - 1) //| Para verificar se todos os caracteres, menos o último, é numérico
    {
        if (!isdigit(value[i]))
            throw std::runtime_error("Configuração inválida: client_max_body_size: é inválido, deve ser um número");
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
        throw std::runtime_error("Configuração inválida: client_max_body_size: é inválido, deve ser um número seguido de unidade");

    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o argumento de max_body_size
    if (tokens.size() == 0 || tokens[0] != ";")
        throw std::runtime_error("Configuração inválida: client_max_body_size: esperava um ponto e vírgula no final de client_max_body_size");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}

void ServerBlock::addErrorPages(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o token 'error_page'
    if (tokens.size() == 0 || tokens[0] == ";")
        throw std::runtime_error("Configuração inválida: error_page: não foi encontrado nenhum error_page");

    std::vector<std::string> codes_str; //| Para armazenar todos os [codes] que possam ter. Exemplo: error_page 101 102 103 page.html
    while (tokens[0] != ";")
    {
        if (tokens[0] == tokens.back())
            throw std::runtime_error("Configuração inválida: error_page: final do arquivo encontrado");
        codes_str.push_back(tokens[0]);
        ParserConfigFile::removeTokens(tokens, 1);
    }

    std::string uri = codes_str.back(); //| O último argumento deve ser a URI
    codes_str.pop_back(); //| Removendo a URI do vetor de [codes]

    std::vector<int> codes;
    for (std::vector<std::string>::iterator it = codes_str.begin(); it != codes_str.end(); ++it)
    {
        if (isAllNumber(*it) == false)
                throw std::runtime_error("Configuração inválida: error_page: [code] é inválido, deve ser um número");

        int code = std::atoi(it->c_str());
        if (code < 100 || code > 599)
            throw std::runtime_error("Configuração inválida: error_page: [code] é inválido, deve ser um número entre 100 e 599");

        codes.push_back(code);
    }

    for (std::vector<int>::iterator it = codes.begin(); it != codes.end(); ++it)
        this->_errorPages[*it] = uri;

    if (tokens.size() == 0) //| Somente por segurança, mas não deve acontecer
        throw std::runtime_error("Configuração inválida: error_page: esperava um ponto e vírgula no final de error_page");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}

void ServerBlock::addLocation(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o token 'location'
    if (tokens.size() == 0)
        throw std::runtime_error("Configuração inválida: location: não foi encontrado nenhum location");

    if (tokens[0][0] != '/')
        throw std::runtime_error("Configuração inválida: location: URI inválida, deve começar com '/'");

    if (this->_locations.count(tokens[0]) > 0)
        throw std::runtime_error("Configuração inválida: location: location duplicado");

    this->_locations[tokens[0]].addLocationBlock(tokens);

    if (tokens.size() == 0)
        throw std::runtime_error("Configuração inválida: location: esperava um ponto e vírgula no final de location");
}

void ServerBlock::addRoot(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o token 'root'
    if (tokens.size() == 0 || tokens[0] == ";")
        throw std::runtime_error("Configuração inválida: root: não foi encontrado nenhuma root");

    if (tokens[0] == tokens.back())
        throw std::runtime_error("Configuração inválida: root: final do arquivo encontrado");

    if (this->_root.first == true) //| Verifica se o root já está definido
        throw std::runtime_error("Configuração inválida: root: root já foi definido");
    this->_root.first = true;

    this->_root.second = tokens[0];

    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o argumento de root
    if (tokens.size() == 0 || tokens[0] != ";")
        throw std::runtime_error("Configuração inválida: root: esperava um ponto e vírgula no final de root");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}
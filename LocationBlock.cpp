#include "LocationBlock.hpp"

LocationBlock::LocationBlock(): _autoIndex(false), _canUpload(false), _uploadPath("./") { this->_index.push_back("index.html"); }

LocationBlock::~LocationBlock() {}

void LocationBlock::addLocationBlock(std::vector<std::string> &tokens)
{
    this->_uri = tokens[0];

    ParserConfigFile::removeTokens(tokens, 2); //| Remove o token de URI e '{'
    ParserConfigFile::verifyToken(tokens, EMPTY, "Configuração inválida: location: não foi encontrado nenhum location");

    while (tokens.size() > 0)
    {
        if (tokens[0] == "autoindex")
			addAutoIndex(tokens);
		else if (tokens[0] == "can_upload")
			addCanUpload(tokens);
		else if (tokens[0] == "alias")
			addAlias(tokens);
		else if (tokens[0] == "return")
			addReturn(tokens);
		else if (tokens[0] == "upload_path")
			addUploadPath(tokens);
		else if (tokens[0] == "index")
			addIndex(tokens);
		else if (tokens[0] == "cgi_extensions")
			addCgiExtensions(tokens);
		else if (tokens[0] == "allow_methods")
			addAllowMethods(tokens);
		else if (tokens[0] == "}")
        {
            ParserConfigFile::removeTokens(tokens, 1);
            break;
        }
		else
			throw std::runtime_error("Configuração inválida: token inválido");
    }

    printLocationBlock();
}

bool LocationBlock::getAutoIndex() const { return this->_autoIndex; }
bool LocationBlock::getCanUpload() const { return this->_canUpload; }
std::string LocationBlock::getUri() const { return this->_uri; }
std::string LocationBlock::getAlias() const { return this->_alias; }
std::string LocationBlock::getReturn() const { return this->_return; }
std::string LocationBlock::getUploadPath() const { return this->_uploadPath; }
std::vector<std::string> LocationBlock::getIndex() const { return this->_index; }
std::vector<std::string> LocationBlock::getCgiExtensions() const { return this->_cgiExtensions; }
std::vector<std::string> LocationBlock::getAllowMethods() const { return this->_allowMethods; }

void LocationBlock::printLocationBlock()
{
    std::cout << "URI: " << this->_uri << std::endl;

    if (this->_autoIndex)
        std::cout << "Autoindex: true" << std::endl;
    else
        std::cout << "Autoindex: false" << std::endl;

    if (this->_canUpload)
        std::cout << "Can upload: true" << std::endl;
    else
        std::cout << "Can upload: false" << std::endl;

    std::cout << "Alias: " << this->_alias << std::endl;

    std::cout << "Return: " << this->_return << std::endl;

    std::cout << "Upload path: " << this->_uploadPath << std::endl;

    std::cout << "Index: " << std::endl;
    for (std::vector<std::string>::iterator it = this->_index.begin(); it != this->_index.end(); ++it)
        std::cout << *it << std::endl;

    std::cout << "CGI extensions: " << std::endl;
    for (std::vector<std::string>::iterator it = this->_cgiExtensions.begin(); it != this->_cgiExtensions.end(); ++it)
        std::cout << *it << std::endl;

    std::cout << "Allow methods: " << std::endl;
    for (std::vector<std::string>::iterator it = this->_allowMethods.begin(); it != this->_allowMethods.end(); ++it)
        std::cout << *it << std::endl;
}

void LocationBlock::addAutoIndex(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1); //| Remove o token 'autoindex'
    ParserConfigFile::verifyToken(tokens, SEMICOLON, "Configuração inválida: autoindex: não foi encontrado nenhum autoindex");

    if (tokens[0] == "on")
        this->_autoIndex = true;
    else if (tokens[0] == "off")
        this->_autoIndex = false;
    else
        throw std::runtime_error("Configuração inválida: autoindex: deve ser 'on' ou 'off'");

    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o argumento de autoindex
    ParserConfigFile::verifyToken(tokens, DIFF_SEMICOLON, "Configuração inválida: autoindex: esperava um ponto e vírgula no final de autoindex");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}

void LocationBlock::addCanUpload(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1); //| Remove o token 'can_upload'
    ParserConfigFile::verifyToken(tokens, SEMICOLON, "Configuração inválida: can_upload: não foi encontrado nenhum can_upload");

    if (tokens[0] == "on")
        this->_canUpload = true;
    else if (tokens[0] == "off")
        this->_canUpload = false;
    else
        throw std::runtime_error("Configuração inválida: can_upload: deve ser 'on' ou 'off'");

    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o argumento de can_upload
    ParserConfigFile::verifyToken(tokens, DIFF_SEMICOLON, "Configuração inválida: can_upload: esperava um ponto e vírgula no final de can_upload");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}

void LocationBlock::addAlias(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1); //| Remove o token 'alias'
    ParserConfigFile::verifyToken(tokens, SEMICOLON, "Configuração inválida: alias: não foi encontrado nenhum alias");

    ParserConfigFile::verifyToken(tokens, END_OF_FILE, "Configuração inválida: alias: final do arquivo encontrado");

    this->_alias = tokens[0];

    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o argumento de alias
    ParserConfigFile::verifyToken(tokens, DIFF_SEMICOLON, "Configuração inválida: alias: esperava um ponto e vírgula no final de alias");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}

void LocationBlock::addReturn(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1); //| Remove o token 'return'
    ParserConfigFile::verifyToken(tokens, SEMICOLON, "Configuração inválida: return: não foi encontrado nenhum return");

    ParserConfigFile::verifyToken(tokens, END_OF_FILE, "Configuração inválida: return: final do arquivo encontrado");

    this->_return = tokens[0];

    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o argumento de return
    ParserConfigFile::verifyToken(tokens, DIFF_SEMICOLON, "Configuração inválida: return: esperava um ponto e vírgula no final de return");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}

void LocationBlock::addUploadPath(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1); //| Remove o token 'upload_path'
    ParserConfigFile::verifyToken(tokens, SEMICOLON, "Configuração inválida: upload_path: não foi encontrado nenhum upload_path");

    ParserConfigFile::verifyToken(tokens, END_OF_FILE, "Configuração inválida: upload_path: final do arquivo encontrado");

    this->_uploadPath = tokens[0];

    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o argumento de upload_path
    ParserConfigFile::verifyToken(tokens, DIFF_SEMICOLON, "Configuração inválida: upload_path: esperava um ponto e vírgula no final de upload_path");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}

void LocationBlock::addIndex(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1); //| Remove o token 'index'
    ParserConfigFile::verifyToken(tokens, SEMICOLON, "Configuração inválida: index: não foi encontrado nenhum index");

    std::vector<std::string> indexes;
    while (tokens[0] != ";")
    {
        ParserConfigFile::verifyToken(tokens, END_OF_FILE, "Configuração inválida: index: final do arquivo encontrado");
        indexes.push_back(tokens[0]);
        ParserConfigFile::removeTokens(tokens, 1);
    }

    for (std::vector<std::string>::iterator it = indexes.begin(); it != indexes.end(); ++it)
        this->_index.push_back(*it);

    ParserConfigFile::verifyToken(tokens, DIFF_SEMICOLON, "Configuração inválida: index: esperava um ponto e vírgula no final de index");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}

void LocationBlock::addCgiExtensions(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1); //| Remove o token 'cgi_extensions'
    ParserConfigFile::verifyToken(tokens, SEMICOLON, "Configuração inválida: cgi_extensions: não foi encontrado nenhum cgi_extensions");

    std::vector<std::string> cgi_extensions;
    while (tokens[0] != ";")
    {
        ParserConfigFile::verifyToken(tokens, END_OF_FILE, "Configuração inválida: cgi_extensions: final do arquivo encontrado");
        if (tokens[0][0] != '.') //| Adiciona o ponto na frente da extensão para ficar .php ou .py
            tokens[0] = "." + tokens[0];
        if (tokens[0] != ".php" && tokens[0] != ".py") //| Um dos bônus: multiplas extensões de cgi
            throw std::runtime_error("Configuração inválida: cgi_extensions: extensão inválida");
        cgi_extensions.push_back(tokens[0]);
        ParserConfigFile::removeTokens(tokens, 1);
    }

    for (std::vector<std::string>::iterator it = cgi_extensions.begin(); it != cgi_extensions.end(); ++it)
        this->_cgiExtensions.push_back(*it);

    ParserConfigFile::verifyToken(tokens, DIFF_SEMICOLON, "Configuração inválida: cgi_extensions: esperava um ponto e vírgula no final de cgi_extensions");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}

void LocationBlock::addAllowMethods(std::vector<std::string> &tokens)
{
    ParserConfigFile::removeTokens(tokens, 1); //| Remove o token 'allow_methods'
    ParserConfigFile::verifyToken(tokens, SEMICOLON, "Configuração inválida: allow_methods: não foi encontrado nenhum allow_methods");

    std::vector<std::string> allow_methods;
    while (tokens[0] != ";")
    {
        ParserConfigFile::verifyToken(tokens, END_OF_FILE, "Configuração inválida: allow_methods: final do arquivo encontrado");
        if (tokens[0] != "GET" && tokens[0] != "POST" && tokens[0] != "DELETE")
            throw std::runtime_error("Configuração inválida: allow_methods: método inválido");
        allow_methods.push_back(tokens[0]);
        ParserConfigFile::removeTokens(tokens, 1);
    }

    for (std::vector<std::string>::iterator it = allow_methods.begin(); it != allow_methods.end(); ++it)
        this->_allowMethods.push_back(*it);

    ParserConfigFile::verifyToken(tokens, DIFF_SEMICOLON, "Configuração inválida: allow_methods: esperava um ponto e vírgula no final de allow_methods");
    ParserConfigFile::removeTokens(tokens, 1); //| Removendo o ponto e vírgula
}
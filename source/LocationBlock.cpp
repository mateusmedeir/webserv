#include "../includes/WebservHeader.hpp"

LocationBlock::LocationBlock(): _autoIndex(false), _canUpload(false), _uploadPath("./") { this->_index.push_back("index.html"); }

LocationBlock::~LocationBlock() {}

void LocationBlock::addLocationBlock(ConfigFile &config)
{
    this->_uri = config.getTokens()[0];
    
    config.removeTokens(2); //| Remove o token de URI e '{'
    config.verifyToken(EMPTY, "Configuração inválida: location: não foi encontrado nenhum location");
    
    while (config.getTokens().size() > 0)
    {
        std::vector<std::string> tokens = config.getTokens();
        if (tokens[0] == "autoindex")
			addAutoIndex(config);
		else if (tokens[0] == "can_upload")
			addCanUpload(config);
		else if (tokens[0] == "alias")
			addAlias(config);
		else if (tokens[0] == "return")
			addReturn(config);
		else if (tokens[0] == "upload_path")
			addUploadPath(config);
		else if (tokens[0] == "index")
			addIndex(config);
		else if (tokens[0] == "cgi_extensions")
			addCgiExtensions(config);
		else if (tokens[0] == "allow_methods")
			addAllowMethods(config);
		else if (tokens[0] == "}")
        {
            config.removeTokens(1);
            break;
        }
		else {
			throw std::runtime_error("Configuração inválida: token inválido");
        }
    }
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

void LocationBlock::addAutoIndex(ConfigFile &config)
{
    config.removeTokens(1); //| Remove o token 'autoindex'
    config.verifyToken(SEMICOLON, "Configuração inválida: autoindex: não foi encontrado nenhum autoindex");

    std::vector<std::string> tokens = config.getTokens();
    if (tokens[0] == "on")
        this->_autoIndex = true;
    else if (tokens[0] == "off")
        this->_autoIndex = false;
    else
        throw std::runtime_error("Configuração inválida: autoindex: deve ser 'on' ou 'off'");

    config.removeTokens(1); //| Removendo o argumento de autoindex
    config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: autoindex: esperava um ponto e vírgula no final de autoindex");
    config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addCanUpload(ConfigFile &config)
{
    config.removeTokens(1); //| Remove o token 'can_upload'
    config.verifyToken(SEMICOLON, "Configuração inválida: can_upload: não foi encontrado nenhum can_upload");

    std::vector<std::string> tokens = config.getTokens();
    if (tokens[0] == "on")
        this->_canUpload = true;
    else if (tokens[0] == "off")
        this->_canUpload = false;
    else
        throw std::runtime_error("Configuração inválida: can_upload: deve ser 'on' ou 'off'");

    config.removeTokens(1); //| Removendo o argumento de can_upload
    config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: can_upload: esperava um ponto e vírgula no final de can_upload");
    config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addAlias(ConfigFile &config)
{
    config.removeTokens(1); //| Remove o token 'alias'
    config.verifyToken(SEMICOLON, "Configuração inválida: alias: não foi encontrado nenhum alias");

    config.verifyToken(END_OF_FILE, "Configuração inválida: alias: final do arquivo encontrado");

    std::vector<std::string> tokens = config.getTokens();
    this->_alias = tokens[0];

    config.removeTokens(1); //| Removendo o argumento de alias
    config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: alias: esperava um ponto e vírgula no final de alias");
    config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addReturn(ConfigFile &config)
{
    config.removeTokens(1); //| Remove o token 'return'
    config.verifyToken(SEMICOLON, "Configuração inválida: return: não foi encontrado nenhum return");

    config.verifyToken(END_OF_FILE, "Configuração inválida: return: final do arquivo encontrado");

    std::vector<std::string> tokens = config.getTokens();
    this->_return = tokens[0];

    config.removeTokens(1); //| Removendo o argumento de return
    config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: return: esperava um ponto e vírgula no final de return");
    config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addUploadPath(ConfigFile &config)
{
    config.removeTokens(1); //| Remove o token 'upload_path'
    config.verifyToken(SEMICOLON, "Configuração inválida: upload_path: não foi encontrado nenhum upload_path");

    config.verifyToken(END_OF_FILE, "Configuração inválida: upload_path: final do arquivo encontrado");

    std::vector<std::string> tokens = config.getTokens();
    this->_uploadPath = tokens[0];

    config.removeTokens(1); //| Removendo o argumento de upload_path
    config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: upload_path: esperava um ponto e vírgula no final de upload_path");
    config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addIndex(ConfigFile &config)
{
    config.removeTokens(1); //| Remove o token 'index'
    config.verifyToken(SEMICOLON, "Configuração inválida: index: não foi encontrado nenhum index");

    std::vector<std::string> indexes;
    while (config.getTokens()[0] != ";")
    {
        config.verifyToken(END_OF_FILE, "Configuração inválida: index: final do arquivo encontrado");
        indexes.push_back(config.getTokens()[0]);
        config.removeTokens(1);
    }

    for (std::vector<std::string>::iterator it = indexes.begin(); it != indexes.end(); ++it)
        this->_index.push_back(*it);

    config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: index: esperava um ponto e vírgula no final de index");
    config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addCgiExtensions(ConfigFile &config)
{
    config.removeTokens(1); //| Remove o token 'cgi_extensions'
    config.verifyToken(SEMICOLON, "Configuração inválida: cgi_extensions: não foi encontrado nenhum cgi_extensions");

    std::vector<std::string> cgi_extensions;
    while (config.getTokens()[0] != ";")
    {
        std::vector<std::string> tokens = config.getTokens();
        config.verifyToken(END_OF_FILE, "Configuração inválida: cgi_extensions: final do arquivo encontrado");
        if (tokens[0][0] != '.') //| Adiciona o ponto na frente da extensão para ficar .php ou .py
            tokens[0] = "." + tokens[0];
        if (tokens[0] != ".php" && tokens[0] != ".py") //| Um dos bônus: multiplas extensões de cgi
            throw std::runtime_error("Configuração inválida: cgi_extensions: extensão inválida");
        cgi_extensions.push_back(tokens[0]);
        config.removeTokens(1);
    }

    for (std::vector<std::string>::iterator it = cgi_extensions.begin(); it != cgi_extensions.end(); ++it)
        this->_cgiExtensions.push_back(*it);

    config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: cgi_extensions: esperava um ponto e vírgula no final de cgi_extensions");
    config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addAllowMethods(ConfigFile &config)
{
    config.removeTokens(1); //| Remove o token 'allow_methods'
    config.verifyToken(SEMICOLON, "Configuração inválida: allow_methods: não foi encontrado nenhum allow_methods");

    std::vector<std::string> allow_methods;
    while (config.getTokens()[0] != ";")
    {
        std::vector<std::string> tokens = config.getTokens();
        config.verifyToken(END_OF_FILE, "Configuração inválida: allow_methods: final do arquivo encontrado");
        if (tokens[0] != "GET" && tokens[0] != "POST" && tokens[0] != "DELETE")
            throw std::runtime_error("Configuração inválida: allow_methods: método inválido");
        allow_methods.push_back(tokens[0]);
        config.removeTokens(1);
    }

    for (std::vector<std::string>::iterator it = allow_methods.begin(); it != allow_methods.end(); ++it)
        this->_allowMethods.push_back(*it);

    config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: allow_methods: esperava um ponto e vírgula no final de allow_methods");
    config.removeTokens(1); //| Removendo o ponto e vírgula
}
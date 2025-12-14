#include "../includes/WebservHeader.hpp"

LocationBlock::LocationBlock(ConfigFile &config) : _config(config), _autoIndex(false), _canUpload(false), _uploadPath("./"), _cookiesEnabled(false) {
    this->_uri = this->_config.getTokens()[0];
    
    this->_config.removeTokens(2); //| Remove o token de URI e '{'
    this->_config.verifyToken(EMPTY, "Configuração inválida: location: não foi encontrado nenhum location");
    
    while (this->_config.getTokens().size() > 0)
    {
        std::vector<std::string> tokens = this->_config.getTokens();
        if (tokens[0] == "autoindex")
			addAutoIndex();
		else if (tokens[0] == "can_upload")
			addCanUpload();
		else if (tokens[0] == "alias")
			addAlias();
		else if (tokens[0] == "return")
			addReturn();
		else if (tokens[0] == "upload_path")
			addUploadPath();
		else if (tokens[0] == "index")
			addIndex();
		else if (tokens[0] == "cgi_extensions")
			addCgiExtensions();
		else if (tokens[0] == "allow_methods")
			addAllowMethods();
		else if (tokens[0] == "cookies_enabled")
			addCookiesEnabled();
		else if (tokens[0] == "}")
        {
            this->_config.removeTokens(1);
            break;
        }
		else {
			throw std::runtime_error("Configuração inválida: token inválido");
        }
    }
}

LocationBlock::~LocationBlock() {}

LocationBlock &LocationBlock::operator=(const LocationBlock &src) {
    if (this != &src) {
        this->_autoIndex = src._autoIndex;
        this->_canUpload = src._canUpload;
        this->_uri = src._uri;
        this->_alias = src._alias;
        this->_return = src._return;
        this->_uploadPath = src._uploadPath;
        this->_index = src._index;
        this->_cgiExtensions = src._cgiExtensions;
        this->_allowMethods = src._allowMethods;
        this->_cookiesEnabled = src._cookiesEnabled;
    }
    return *this;
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
bool LocationBlock::getCookiesEnabled() const { return this->_cookiesEnabled; }

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

void LocationBlock::addAutoIndex()
{
    this->_config.removeTokens(1); //| Remove o token 'autoindex'
    this->_config.verifyToken(SEMICOLON, "Configuração inválida: autoindex: não foi encontrado nenhum autoindex");

    std::vector<std::string> tokens = this->_config.getTokens();
    if (tokens[0] == "on")
        this->_autoIndex = true;
    else if (tokens[0] == "off")
        this->_autoIndex = false;
    else
        throw std::runtime_error("Configuração inválida: autoindex: deve ser 'on' ou 'off'");

    this->_config.removeTokens(1); //| Removendo o argumento de autoindex
    this->_config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: autoindex: esperava um ponto e vírgula no final de autoindex");
    this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addCanUpload()
{
    this->_config.removeTokens(1); //| Remove o token 'can_upload'
    this->_config.verifyToken(SEMICOLON, "Configuração inválida: can_upload: não foi encontrado nenhum can_upload");

    std::vector<std::string> tokens = this->_config.getTokens();
    if (tokens[0] == "on")
        this->_canUpload = true;
    else if (tokens[0] == "off")
        this->_canUpload = false;
    else
        throw std::runtime_error("Configuração inválida: can_upload: deve ser 'on' ou 'off'");

    this->_config.removeTokens(1); //| Removendo o argumento de can_upload
    this->_config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: can_upload: esperava um ponto e vírgula no final de can_upload");
    this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addAlias()
{
    this->_config.removeTokens(1); //| Remove o token 'alias'
    this->_config.verifyToken(SEMICOLON, "Configuração inválida: alias: não foi encontrado nenhum alias");

    this->_config.verifyToken(END_OF_FILE, "Configuração inválida: alias: final do arquivo encontrado");

    std::vector<std::string> tokens = this->_config.getTokens();
    this->_alias = tokens[0];

    this->_config.removeTokens(1); //| Removendo o argumento de alias
    this->_config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: alias: esperava um ponto e vírgula no final de alias");
    this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addReturn()
{
    this->_config.removeTokens(1); //| Remove o token 'return'
    this->_config.verifyToken(SEMICOLON, "Configuração inválida: return: não foi encontrado nenhum return");

    this->_config.verifyToken(END_OF_FILE, "Configuração inválida: return: final do arquivo encontrado");

    std::vector<std::string> tokens = this->_config.getTokens();
    this->_return = tokens[0];

    this->_config.removeTokens(1); //| Removendo o argumento de return
    this->_config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: return: esperava um ponto e vírgula no final de return");
    this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addUploadPath()
{
    this->_config.removeTokens(1); //| Remove o token 'upload_path'
    this->_config.verifyToken(SEMICOLON, "Configuração inválida: upload_path: não foi encontrado nenhum upload_path");

    this->_config.verifyToken(END_OF_FILE, "Configuração inválida: upload_path: final do arquivo encontrado");

    std::vector<std::string> tokens = this->_config.getTokens();
    this->_uploadPath = tokens[0];

    this->_config.removeTokens(1); //| Removendo o argumento de upload_path
    this->_config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: upload_path: esperava um ponto e vírgula no final de upload_path");
    this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addIndex()
{
    this->_config.removeTokens(1); //| Remove o token 'index'
    this->_config.verifyToken(SEMICOLON, "Configuração inválida: index: não foi encontrado nenhum index");

    std::vector<std::string> indexes;
    while (this->_config.getTokens()[0] != ";")
    {
        this->_config.verifyToken(END_OF_FILE, "Configuração inválida: index: final do arquivo encontrado");
        indexes.push_back(this->_config.getTokens()[0]);
        this->_config.removeTokens(1);
    }

    for (std::vector<std::string>::iterator it = indexes.begin(); it != indexes.end(); ++it)
        this->_index.push_back(*it);

    this->_config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: index: esperava um ponto e vírgula no final de index");
    this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addCgiExtensions()
{
    this->_config.removeTokens(1); //| Remove o token 'cgi_extensions'
    this->_config.verifyToken(SEMICOLON, "Configuração inválida: cgi_extensions: não foi encontrado nenhum cgi_extensions");

    std::vector<std::string> cgi_extensions;
    while (this->_config.getTokens()[0] != ";")
    {
        std::vector<std::string> tokens = this->_config.getTokens();
        this->_config.verifyToken(END_OF_FILE, "Configuração inválida: cgi_extensions: final do arquivo encontrado");
        if (tokens[0][0] != '.') //| Adiciona o ponto na frente da extensão para ficar .php ou .py
            tokens[0] = "." + tokens[0];
        if (tokens[0] != ".php" && tokens[0] != ".py") //| Um dos bônus: multiplas extensões de cgi
            throw std::runtime_error("Configuração inválida: cgi_extensions: extensão inválida");
        cgi_extensions.push_back(tokens[0]);
        this->_config.removeTokens(1);
    }

    for (std::vector<std::string>::iterator it = cgi_extensions.begin(); it != cgi_extensions.end(); ++it)
        this->_cgiExtensions.push_back(*it);

    this->_config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: cgi_extensions: esperava um ponto e vírgula no final de cgi_extensions");
    this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addAllowMethods()
{
    this->_config.removeTokens(1); //| Remove o token 'allow_methods'
    this->_config.verifyToken(SEMICOLON, "Configuração inválida: allow_methods: não foi encontrado nenhum allow_methods");

    std::vector<std::string> allow_methods;
    while (this->_config.getTokens()[0] != ";")
    {
        std::vector<std::string> tokens = this->_config.getTokens();
        this->_config.verifyToken(END_OF_FILE, "Configuração inválida: allow_methods: final do arquivo encontrado");
        if (tokens[0] != "GET" && tokens[0] != "POST" && tokens[0] != "DELETE")
            throw std::runtime_error("Configuração inválida: allow_methods: método inválido");
        allow_methods.push_back(tokens[0]);
        this->_config.removeTokens(1);
    }

    for (std::vector<std::string>::iterator it = allow_methods.begin(); it != allow_methods.end(); ++it)
        this->_allowMethods.push_back(*it);

    this->_config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: allow_methods: esperava um ponto e vírgula no final de allow_methods");
    this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}

void LocationBlock::addCookiesEnabled()
{
    this->_config.removeTokens(1); //| Remove o token 'cookies_enabled'
    this->_config.verifyToken(SEMICOLON, "Configuração inválida: cookies_enabled: não foi encontrado nenhum cookies_enabled");

    std::vector<std::string> tokens = this->_config.getTokens();
    if (tokens[0] == "on")
        this->_cookiesEnabled = true;
    else if (tokens[0] == "off")
        this->_cookiesEnabled = false;
    else
        throw std::runtime_error("Configuração inválida: cookies_enabled: deve ser 'on' ou 'off'");

    this->_config.removeTokens(1); //| Removendo o argumento de cookies_enabled
    this->_config.verifyToken(DIFF_SEMICOLON, "Configuração inválida: cookies_enabled: esperava um ponto e vírgula no final de cookies_enabled");
    this->_config.removeTokens(1); //| Removendo o ponto e vírgula
}

bool LocationBlock::validatePath(const std::string &path) const {
    bool isValid = false;
    std::ifstream file(path.c_str(), std::ios::binary);

    isValid = file.good();
    file.close();
    return isValid;
}

std::string LocationBlock::getPath(const std::string &root, const std::string &requestUri) const
{
    std::string locationUri = this->getUri(); // "/"
    std::string relativeUri = requestUri;

    // remove o prefixo da location
    if (relativeUri.find(locationUri) == 0)
        relativeUri = relativeUri.substr(locationUri.size());

    std::string fullPath = root;
    if (!fullPath.empty() && fullPath[fullPath.size() - 1] != '/')
        fullPath += "/";

    fullPath += relativeUri;

    // se terminar com / → tenta index
    if (fullPath[fullPath.size() - 1] == '/') {
        std::vector<std::string> indexes = getIndex();
        if (indexes.empty())
            indexes.push_back("index.html");

        for (size_t i = 0; i < indexes.size(); i++) {
            std::string test = fullPath + indexes[i];
            if (validatePath(test))
                return test;
        }
        return "";
    }

    // arquivo direto
    if (validatePath(fullPath))
        return fullPath;

    return "";
}
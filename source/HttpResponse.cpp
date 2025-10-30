#include "../includes/WebservHeader.hpp"

HttpResponse::HttpResponse(){
	this->_http_version = "HTTP/1.0";
	this->_status_code = 200;
	this->_status_message = "OK";
};

HttpResponse::~HttpResponse(){};

void HttpResponse::handleGet(const HttpRequest &req) {
	// Verificar se é uma requisição CGI
	if (isCgiRequest(req.getUri())) {
		std::string scriptPath = getCgiScriptPath(req.getUri());
		return handleCgi(req, scriptPath);
	}
	
	std::string path = uriToPath(req.getUri());

	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file) {
		this->setStatus(404, "Not Found");
		this->setBody("<h1>404 Not Found</h1>", "text/html");
		return;
	}

	std::ostringstream buffer;
	buffer << file.rdbuf();
	this->setStatus(200, "OK");
	
	// Detectar MIME type automaticamente
	std::string mimeType = getMimeType(path);
	this->setBody(buffer.str(), mimeType);
};

void HttpResponse::handlePost(const HttpRequest &req){
	// Verificar se é uma requisição CGI
	if (isCgiRequest(req.getUri())) {
		std::string scriptPath = getCgiScriptPath(req.getUri());
		return handleCgi(req, scriptPath);
	}
	
	std::string path = "./uploads/upload.txt";

	std::ofstream file(path.c_str());
	if (!file) {
		this->setStatus(500, "Internal Server Error");
		this->setBody("<h1>500 Internal Server Error</h1>", "text/html");
		return;
	}

	file << req.getBody();
	file.close();

	this->setStatus(201, "Created");
	this->setBody("<h1>File uploaded successfully!</h1>", "text/html");
};

void HttpResponse::handleDelete(const HttpRequest &req){
		std::string path = uriToPath(req.getUri());

		if (std::remove(path.c_str()) == 0) {
			this->setStatus(200, "OK");
			this->setBody("<h1>File deleted successfully</h1>", "text/html");
		} else {
			this->setStatus(404, "Not Found");
			this->setBody("<h1>404 Not Found</h1>", "text/html");
		}
};

void HttpResponse::dispatchRequest(const HttpRequest &req){
	std::cout << "Dispatching request for method: " << req.getMethod() << std::endl;
	if (this->_status_code != 200)
		return;

	if(req.getMethod() == "GET")
		return handleGet(req);
	else if(req.getMethod() == "POST")
		return handlePost(req);
	else if(req.getMethod() == "DELETE")
		return handleDelete(req);
	else 
		this->setErrorPage(405);
}

void		HttpResponse::setStatus(int code, const std::string &message){
	this->_status_code = code;
	this->_status_message = message;
};

void		HttpResponse::setHeader(const std::string &key, const std::string &value){
	this->_headers[key] = value;
};

void		HttpResponse::setBody(const std::string &b, const std::string &contentType){
	_body = b;
	this->_headers["Content-Type"] = contentType;
	this->_headers["Content-Length"] = intToString(_body.size());
}

std::string	HttpResponse::toString() const{
	std::ostringstream response;

	// Status line
	response << _http_version << " "
				<< _status_code << " "
				<< _status_message << "\r\n";

	// Headers
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
			it != _headers.end(); ++it) {
		response << it->first << ": " << it->second << "\r\n";
	}

	// Linha em branco
	response << "\r\n";

	// Corpo
	response << _body;

	return response.str();
}

std::string	HttpResponse::intToString(int n) const{
	std::ostringstream oss;
	oss << n;
	return oss.str();
}

std::string HttpResponse::uriToPath(const std::string &uri) const {
    std::string path = uri;

    // Se termina com '/', tenta index.html
    if (path[path.size() - 1] == '/') {
        path += "index.html";
    }
    if (path[0] != '/')
        path = "/" + path;

    std::string fullPath = "./www" + path;
    std::cout << "Converted URI to path: " << fullPath << std::endl;
    return fullPath;
}

/**
 * @brief Detecta o MIME type baseado na extensão do arquivo
 */
std::string HttpResponse::getMimeType(const std::string &path) const {
    // Encontrar a extensão
    size_t dotPos = path.rfind('.');
    if (dotPos == std::string::npos) {
        return "application/octet-stream"; // Tipo binário genérico
    }
    
    std::string ext = path.substr(dotPos);
    
    // HTML e XML
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".xml") return "application/xml";
    
    // Texto
    if (ext == ".txt") return "text/plain";
    if (ext == ".css") return "text/css";
    
    // JavaScript
    if (ext == ".js") return "application/javascript";
    if (ext == ".json") return "application/json";
    
    // Imagens
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif") return "image/gif";
    if (ext == ".svg") return "image/svg+xml";
    if (ext == ".ico") return "image/x-icon";
    if (ext == ".webp") return "image/webp";
    
    // Fontes
    if (ext == ".woff") return "font/woff";
    if (ext == ".woff2") return "font/woff2";
    if (ext == ".ttf") return "font/ttf";
    if (ext == ".otf") return "font/otf";
    
    // Documentos
    if (ext == ".pdf") return "application/pdf";
    if (ext == ".zip") return "application/zip";
    if (ext == ".tar") return "application/x-tar";
    if (ext == ".gz") return "application/gzip";
    
    // Vídeo
    if (ext == ".mp4") return "video/mp4";
    if (ext == ".webm") return "video/webm";
    if (ext == ".avi") return "video/x-msvideo";
    
    // Áudio
    if (ext == ".mp3") return "audio/mpeg";
    if (ext == ".wav") return "audio/wav";
    if (ext == ".ogg") return "audio/ogg";
    
    // Default
    return "application/octet-stream";
}

void		HttpResponse::setErrorPage(int code){
	std::string path = "./error_pages/" + intToString(code) + ".html";
	std::ifstream file(path.c_str());
	if(!file){
		setStatus(code, "Error");
		setBody("<h1>"+ intToString(code) + "Error</h1>","text/html");
		return;
	}
	std::ostringstream buffer;
	buffer << file.rdbuf();
	setStatus(code, "Error");
	setBody(buffer.str(),"text/html");
}

/**
 * @brief Verifica se a URI é uma requisição CGI (.py ou .php)
 */
bool HttpResponse::isCgiRequest(const std::string &uri) const {
	// Remover query string se houver
	std::string path = uri;
	size_t qPos = path.find('?');
	if (qPos != std::string::npos) {
		path = path.substr(0, qPos);
	}
	
	// Verificar extensão
	if (path.length() >= 3 && path.substr(path.length() - 3) == ".py")
		return true;
	if (path.length() >= 4 && path.substr(path.length() - 4) == ".php")
		return true;
	
	return false;
}

/**
 * @brief Converte URI em caminho do script CGI
 */
std::string HttpResponse::getCgiScriptPath(const std::string &uri) const {
	// Remover query string
	std::string path = uri;
	size_t qPos = path.find('?');
	if (qPos != std::string::npos) {
		path = path.substr(0, qPos);
	}
	
	// Converter para caminho do filesystem
	// Assumindo que scripts CGI estão em ./www/cgi-bin/
	if (path[0] != '/')
		path = "/" + path;
	
	std::string fullPath = "./www" + path;
	std::cout << "[HttpResponse] CGI script path: " << fullPath << std::endl;
	return fullPath;
}

/**
 * @brief Prepara execução assíncrona de CGI
 * 
 * NOTA: Esta versão não bloqueia! O Client gerencia o CGI de forma assíncrona.
 * Retorna o caminho do script para o Client iniciar a execução.
 */
void HttpResponse::handleCgi(const HttpRequest &req, const std::string &scriptPath) {
	std::cout << "[HttpResponse] Preparing async CGI for: " << scriptPath << std::endl;
	
	// Verificar se o script existe
	std::ifstream scriptFile(scriptPath.c_str());
	if (!scriptFile) {
		std::cerr << "[HttpResponse] CGI script not found: " << scriptPath << std::endl;
		this->setStatus(404, "Not Found");
		this->setBody("<h1>404 Not Found</h1><p>CGI script not found</p>", "text/html");
		return;
	}
	scriptFile.close();
	
	// ✅ NOVA VERSÃO: Não faz nada aqui!
	// O Client vai chamar startCgiExecution() e gerenciar tudo de forma assíncrona
	// Este método apenas valida que o script existe
	
	std::cout << "[HttpResponse] CGI validation OK, client will handle execution" << std::endl;
	
	(void)req;  // Silenciar warning de variável não usada
}
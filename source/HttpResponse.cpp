#include "../includes/WebservHeader.hpp"
#include "../includes/CookieHandler.hpp"
#include "../includes/CgiHandler.hpp"

HttpResponse::HttpResponse(){
	this->_http_version = "HTTP/1.0";
	this->_status_code = 200;
	this->_status_message = "OK";
};

HttpResponse::~HttpResponse(){};

void HttpResponse::handleGet(const HttpRequest &req) {
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
	std::string mimeType = getMimeType(path);
	this->setBody(buffer.str(), mimeType);
};

void HttpResponse::handlePost(const HttpRequest &req){
	std::string path = "./uploads/upload.txt";

	std::ofstream file(path.c_str());
	if (!file) {
		this->setStatus(500, "Internal Server Error");
		this->setBody("<h1>500 Internal Server Error</h1>", "text/html");
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
    
    // Remover query string se houver
    size_t queryPos = path.find('?');
    if (queryPos != std::string::npos) {
        path = path.substr(0, queryPos);
    }

    if (path[path.size() - 1] == '/') {
        path += "index.html";
    }
    if (path[0] != '/') {
        path = "/" + path;
    }

    std::cout << "Converted URI to path: " << "./www" + path << std::endl;
    return "./www" + path;
}

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

void HttpResponse::processCookies(const HttpRequest &req, const LocationBlock &location) {
	if (location.getCookiesEnabled()) {
		CookieHandler::handleCookie(*this, req);
	}
}

void HttpResponse::dispatchRequest(const HttpRequest &req, const ServerBlock &serverBlock) {
	std::cout << "Dispatching request for method: " << req.getMethod() << std::endl;
	if (this->_status_code != 200)
		return;
	
	// Encontrar melhor match de location
	std::map<std::string, LocationBlock> locations = serverBlock.getLocations();
	std::string bestMatch = "";
	const LocationBlock* locationPtr = NULL;
	
	for (std::map<std::string, LocationBlock>::const_iterator it = locations.begin();
		 it != locations.end(); ++it) {
		const std::string &path = it->first;
		if (req.getUri().compare(0, path.size(), path) == 0) {
			if (path.size() > bestMatch.size()) {
				bestMatch = path;
				locationPtr = &(it->second);
			}
		}
	}
	
	// Verificar se precisa executar CGI
	if (!bestMatch.empty() && locationPtr && CgiHandler::shouldExecuteCgi(req.getUri(), *locationPtr)) {
		// Executar CGI
		std::string cgiOutput;
		
		if (CgiHandler::executeCgi(req, serverBlock, *locationPtr, cgiOutput)) {
			processCgiResponse(cgiOutput);
		} else {
			setStatus(502, "Bad Gateway");
			setBody("<h1>502 Bad Gateway</h1>", "text/html");
		}
		return;
	}
	
	// Processamento normal
	if(req.getMethod() == "GET")
		return handleGet(req);
	else if(req.getMethod() == "POST")
		return handlePost(req);
	else if(req.getMethod() == "DELETE")
		return handleDelete(req);
	else 
		this->setErrorPage(405);
}

bool HttpResponse::dispatchRequestAsync(const HttpRequest &req, const ServerBlock &serverBlock, int clientFd) {
	std::cout << "Dispatching request async for method: " << req.getMethod() << std::endl;
	if (this->_status_code != 200)
		return true; // Resposta pronta (erro)
	
	// Encontrar melhor match de location
	std::map<std::string, LocationBlock> locations = serverBlock.getLocations();
	std::string bestMatch = "";
	const LocationBlock* locationPtr = NULL;
	
	for (std::map<std::string, LocationBlock>::const_iterator it = locations.begin();
		 it != locations.end(); ++it) {
		const std::string &path = it->first;
		if (req.getUri().compare(0, path.size(), path) == 0) {
			if (path.size() > bestMatch.size()) {
				bestMatch = path;
				locationPtr = &(it->second);
			}
		}
	}
	
	// Verificar se precisa executar CGI
	if (!bestMatch.empty() && locationPtr && CgiHandler::shouldExecuteCgi(req.getUri(), *locationPtr)) {
		// Executar CGI assíncrono
		if (CgiHandler::executeCgiAsync(req, serverBlock, *locationPtr, clientFd)) {
			return false; // Resposta não está pronta - será processada assincronamente
		} else {
			setStatus(502, "Bad Gateway");
			setBody("<h1>502 Bad Gateway</h1>", "text/html");
			return true; // Resposta pronta (erro)
		}
	}
	
	// Processamento normal (síncrono)
	dispatchRequest(req, serverBlock);
	return true; // Resposta pronta
}

void HttpResponse::processCgiResponse(const std::string& cgiOutput) {
	size_t headerEnd = cgiOutput.find("\r\n\r\n");
	
	if (headerEnd == std::string::npos) {
		// Não encontrou separador - tratar como erro
		setStatus(502, "Bad Gateway");
		setBody("<h1>502 Bad Gateway</h1>", "text/html");
		return;
	}
	
	// Extrair headers
	std::string headersStr = cgiOutput.substr(0, headerEnd);
	std::string body = cgiOutput.substr(headerEnd + 4);
	
	// Parsear headers
	std::istringstream headerStream(headersStr);
	std::string line;
	
	while (std::getline(headerStream, line)) {
		if (line.empty() || line == "\r")
			break;
		
		// Remover \r
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		
		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos) {
			std::string key = line.substr(0, colonPos);
			std::string value = line.substr(colonPos + 1);
			
			// Remover espaços do início do value
			while (!value.empty() && value[0] == ' ')
				value.erase(0, 1);
			
			// Status especial
			std::string lowerKey = key;
			for (size_t i = 0; i < lowerKey.size(); i++) {
				lowerKey[i] = std::tolower(lowerKey[i]);
			}
			
			if (lowerKey == "status") {
				int statusCode = std::atoi(value.substr(0, 3).c_str());
				std::string statusMsg = value.substr(4);
				setStatus(statusCode, statusMsg);
			} else {
				setHeader(key, value);
			}
		}
	}
	
	// Se não há status code nos headers, usar padrão
	if (this->_status_code == 200) {
		setStatus(200, "OK");
	}
	
	// Adicionar body
	std::string contentType = getHeaderValue("Content-Type");
	if (contentType.empty()) {
		contentType = "text/html";
	}
	setBody(body, contentType);
}

std::string HttpResponse::findBestLocationMatch(const std::string& uri,
                                                   const ServerBlock& serverBlock,
                                                   LocationBlock& location) const {
	std::map<std::string, LocationBlock> locations = serverBlock.getLocations();
	std::string bestMatch = "";
	
	for (std::map<std::string, LocationBlock>::const_iterator it = locations.begin();
		 it != locations.end(); ++it) {
		const std::string &path = it->first;
		if (uri.compare(0, path.size(), path) == 0) {
			if (path.size() > bestMatch.size()) {
				bestMatch = path;
				location = it->second;
			}
		}
	}
	
	return bestMatch;
}

int HttpResponse::getStatusCode() const {
	return _status_code;
}

std::string HttpResponse::getStatusMessage() const {
	return _status_message;
}

std::string HttpResponse::getHeaderValue(const std::string &key) const {
	std::string lowerKey = key;
	for (size_t i = 0; i < lowerKey.size(); ++i) {
		lowerKey[i] = std::tolower(lowerKey[i]);
	}
	
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
		 it != _headers.end(); ++it) {
		std::string lowerHeader = it->first;
		for (size_t i = 0; i < lowerHeader.size(); ++i) {
			lowerHeader[i] = std::tolower(lowerHeader[i]);
		}
		if (lowerHeader == lowerKey) {
			return it->second;
		}
	}
	return "";
}
#include "../includes/WebservHeader.hpp"
#include "../includes/CookieHandler.hpp"

HttpResponse::HttpResponse(){
	this->_http_version = "HTTP/1.0";
	this->_status_code = 200;
	this->_status_message = "OK";
	this->_execAutoIndex = false;
	this->sended = false;
};

HttpResponse::~HttpResponse(){};

void	HttpResponse::generateAutoIndexHTML(const HttpRequest &req, const ServerBlock &serverBlock, const LocationBlock &location) {
	std::stringstream output;
	std::string path = location.getPath(serverBlock.getRoot().second, req.getUri());
	Logger::debug("PAth dentro do generateAutoIndex: " + path);
	if (path.empty()) return setResponseByStatus(404, &serverBlock);
	DIR* dir = opendir(path.c_str());
	if (dir == NULL) {
		return;
	}
	output << "<html><head><title>Autoindex</title></head><body><h1>Autoindex</h1><ul>";
	dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;
		if (name != "." && name != "..") {
			output << "<li>";
			//Checando se e um diretorio
			if (entry->d_type == DT_DIR) {
				//se for diretorio, criar um link direto para ele
				output << "<a href=\"" << name << "/\">" << name << " (directory)</a>";
			} else {
				// Para arquivos, gera um link para download
				output << "<a href=\"" << name << "\">" << name << "</a>";
			}
			output << "</li>";
		}
	}
	output << "</ul></body></html>";
	closedir(dir);
	setResponseByStatus(200, &serverBlock, output.str());
}

void HttpResponse::handleGet(const HttpRequest &req, const ServerBlock &serverBlock, const LocationBlock &location){
	if (!location.getReturn().empty()) return setResponseByStatus(302, &serverBlock, location.getReturn());

	std::string path = location.getPath(serverBlock.getRoot().second, req.getUri());
	path = extractAndDecodeUri(path);
	Logger::debug("Path dentro do delete: " + path);
	if (path.empty()) return setResponseByStatus(404, &serverBlock);

	// validar se e autoindex.
	if (this->getExecAutoIndex()) {
		// Execute autoindex...
		Logger::debug("EXECUTANDO AUTOINDEX....");
		this->generateAutoIndexHTML(req, serverBlock, location);
		return ;
	}
	std::vector<std::string> locationIndexes = location.getIndex();
	for (std::vector<std::string>::iterator it = locationIndexes.begin(); it != locationIndexes.end(); it++) {
		if (access((path + *it).c_str(), R_OK) == 0) {
			// rootOrAliasPlusUri += *it;
			// this->request.setUri(this->request.getUri() + *it);
			path += *it;
		}
	}
	Logger::debug("Path executado no GET: " + path);
	if (access(path.c_str(), R_OK) != 0) {
		return setResponseByStatus(403, &serverBlock);
	}
	
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file) return setResponseByStatus(404, &serverBlock);
	
	Logger::debug("EXECUTANDO O GET METODO....");
	std::ostringstream buffer;
	buffer << file.rdbuf();

	setResponseByStatus(200, &serverBlock, buffer.str(), getMimeType(path));
};

void HttpResponse::handleDelete(const HttpRequest &req, const ServerBlock &serverBlock, const LocationBlock &location){
	(void)serverBlock;
	std::string locationUploadDir = location.getUploadPath();
	if (locationUploadDir.empty()) return (setResponseByStatus(404, &serverBlock));

	std::string uri = extractAndDecodeUri(req.getUri());
	if (uri[uri.size() - 1] == '/') return (setResponseByStatus(404, &serverBlock));

	size_t	filePos = uri.rfind('/');
	std::string fileName = uri.substr(filePos);
	std::string path = "";

	if (locationUploadDir[locationUploadDir.size() - 1] == '/')
		path = locationUploadDir + fileName;
	else
		path = locationUploadDir + "/" + fileName;

	// std::cout << serverBlock.getRoot().second << std::endl;
	// std::cout << uri << std::endl;
	// std::string path = location.getPath(location.getUploadPath(), uri);
	Logger::debug("Path dentro do delete: " + path);
	if (std::remove(path.c_str()) == 0) {
		setResponseByStatus(200, &serverBlock);
	} else {
		setResponseByStatus(404, &serverBlock);
	}
};

void HttpResponse::dispatchRequest(Client *client, const ServerBlock &serverBlock, const LocationBlock &location) {
	HttpRequest req = client->request;
	if (this->_status_code != 200)
	return;
	
	processCookies(req, location);

	if (!location.getReturn().empty()) {
		setResponseByStatus(302, &serverBlock, location.getReturn());
		return;
	}
	if (req.getIsCgi()) {
		Logger::debug("Dispatching to CGI handler for URI: " + req.getUri());

		client->cgiHandler = new CgiHandler(req, serverBlock, location);
		if (!client->cgiHandler->start()) {
				Logger::error("Failed to start CGI handler.");
				setErrorPage(500, &serverBlock);

				delete client->cgiHandler;
				client->cgiHandler = NULL;
				return;
		}
		client->setState(WAITING_CGI);
		return;
	}

	if(req.getMethod() == "GET")
		return handleGet(req, serverBlock, location);
	else if(req.getMethod() == "POST")
		return handlePost(req, serverBlock, location);
	else if(req.getMethod() == "DELETE")
		return handleDelete(req, serverBlock, location);
	else 
		return setResponseByStatus(405, &serverBlock);
}

void HttpResponse::handlePost(const HttpRequest &req, const ServerBlock &serverBlock, const LocationBlock &location){
	std::string locationUploadDir = location.getUploadPath();
	std::string fullPath = "";
	if (locationUploadDir.empty()) return (setResponseByStatus(403, &serverBlock));
	if (locationUploadDir[locationUploadDir.size() - 1] == '/')
		fullPath = locationUploadDir + req.getUploadFileName();
	else
		fullPath = locationUploadDir + "/" + req.getUploadFileName();
	Logger::debug("Path dentro do post: " + fullPath);
	std::ofstream	newFile(fullPath.c_str());
	if (!newFile.is_open()) {
		Logger::error("Nao foi possivel criar o arquivo.");
		return this->setResponseByStatus(400, &serverBlock);
	}
	size_t endHeader = req.getBody().find("\r\n\r\n") + 4;
	size_t endFormData = req.getBody().find(req.getEndBoudary());
	newFile << req.getBody().substr(endHeader, endFormData - endHeader - 2);
	newFile.close();
	setResponseByStatus(201, &serverBlock);
	this->setHeader("Access-Control-Allow-Origin", "*");
	this->setBody("<h1>File named " + req.getUploadFileName() + " was uploaded successfully!</h1>", "text/html");
};

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

std::string HttpResponse::getMimeType(const std::string &path) const {
    // Encontrar a extensão
    size_t dotPos = path.rfind('.');
    if (dotPos == std::string::npos) {
        return "application/octet-stream"; // Tipo binário genérico
    }
    
    std::string ext = path.substr(dotPos);
    
    // HTML e XML
    if (ext == ".html" || ext == ".htm" || ext == ".php") return "text/html";
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

std::string HttpResponse::getStatusMessageForCode(int code) const {
	switch (code) {
		//| 2xx Success
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		
		//| 3xx Redirection
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 303: return "See Other";
		case 304: return "Not Modified";
		case 307: return "Temporary Redirect";
		case 308: return "Permanent Redirect";
		
		//| 4xx Client Errors
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 406: return "Not Acceptable";
		case 408: return "Request Timeout";
		case 409: return "Conflict";
		case 410: return "Gone";
		case 411: return "Length Required";
		case 413: return "Content Too Large";
		case 414: return "URI Too Long";
		case 415: return "Unsupported Media Type";
		case 418: return "I'm a teapot";
		case 422: return "Unprocessable Content";
		case 429: return "Too Many Requests";
		
		//| 5xx Server Errors
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		case 504: return "Gateway Timeout";
		case 505: return "HTTP Version Not Supported";
		
		default: return "Error";
	}
}

void		HttpResponse::setErrorPage(int code, const ServerBlock *serverBlock){
	std::string path;
	if (!_body.empty()) return;
	//| Verificar se existe página personalizada no ServerBlock
	if (serverBlock) {
		std::map<int, std::string> errorPages = serverBlock->getErrorPages();
		std::map<int, std::string>::const_iterator it = errorPages.find(code);
		if (it != errorPages.end()) {
			//| Combinar root com o URI da página de erro configurada
			std::string root = serverBlock->getRoot().second;
			if (root.empty())
				root = ".";
			path = root + it->second;
		}
	}
	
	//| Se não encontrou página personalizada, usar o fallback padrão
	if (path.empty()) {
		path = "./error_pages/" + intToString(code) + ".html";
	}
	
	//| Definir a mensagem de status apropriada
	std::string statusMessage = getStatusMessageForCode(code);
	
	std::ifstream file(path.c_str());
	if (!file) {
		//| Se não conseguiu abrir o arquivo, tentar fallback padrão
		if (path.find("./error_pages/") == std::string::npos) {
			path = "./error_pages/" + intToString(code) + ".html";
			file.open(path.c_str());
		}
		if (!file) {
			//| Se ainda não conseguiu, gerar página de erro genérica
			setStatus(code, statusMessage);
			setBody("<html><head><title>" + intToString(code) + " " + statusMessage + "</title></head>"
					"<body><h1>" + intToString(code) + " " + statusMessage + "</h1></body></html>", "text/html");
			return;
		}
	}
	std::ostringstream buffer;
	buffer << file.rdbuf();
	setStatus(code, statusMessage);
	setBody(buffer.str(), "text/html");
}

void HttpResponse::setResponseByStatus(int statusCode, const ServerBlock *serverBlock, const std::string &bodyContent, const std::string &contentType) {
	if (statusCode >= 400) {
		setErrorPage(statusCode, serverBlock);
	} else if (statusCode == 302) {
		setStatus(302, "Found");
		setHeader("Location", bodyContent);
		setBody("<h1>302 Found</h1>", "text/html");
	} else {
		std::string statusMessage = getStatusMessageForCode(statusCode);
		setStatus(statusCode, statusMessage);
		setBody(bodyContent, contentType);
	}
}

void HttpResponse::processCookies(const HttpRequest &req, const LocationBlock &location) {
	if (location.getCookiesEnabled()) {
		CookieHandler::handleCookie(*this, req);
	}
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

// std::string 	getHttpVersion() const;
std::string HttpResponse::getHttpVersion() const {
	return _http_version;
}

int HttpResponse::getStatusCode() const {
	return _status_code;
}

std::string HttpResponse::getStatusMessage() const {
	return _status_message;
}

void HttpResponse::setExecAutoIndex(bool exec) {
	this->_execAutoIndex = exec;
}

bool HttpResponse::getExecAutoIndex() const {
	return (this->_execAutoIndex);
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

void HttpResponse::parseCgiOutput(const std::string& cgiRawOutput) {
    // Clear existing response data
    this->_headers.clear();
    this->_body.clear();
    this->_status_code = 200; // Default status
    this->_status_message = "OK"; // Default message

    size_t headerEndPos = cgiRawOutput.find("\r\n\r\n");
    std::string headersPart;
    std::string bodyPart;

    if (headerEndPos != std::string::npos) {
        headersPart = cgiRawOutput.substr(0, headerEndPos);
        bodyPart = cgiRawOutput.substr(headerEndPos + 4);
    } else {
        // No header-body separator found, treat entire output as body
        bodyPart = cgiRawOutput;
    }

    std::istringstream issHeaders(headersPart);
    std::string line;
    while (std::getline(issHeaders, line) && !line.empty()) {
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line.resize(line.length() - 1); // Remove trailing \r (C++98 compatible)
        }
        if (line.empty()) continue; // Skip empty lines

        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // Trim leading whitespace from value
            size_t firstChar = value.find_first_not_of(" \t");
            if (firstChar != std::string::npos) {
                value = value.substr(firstChar);
            }

            if (key == "Status") {
                // Parse status code and message
                size_t spacePos = value.find(' ');
                if (spacePos != std::string::npos) {
                    this->_status_code = std::atoi(value.substr(0, spacePos).c_str());
                    this->_status_message = value.substr(spacePos + 1);
                } else {
                    this->_status_code = std::atoi(value.c_str());
                    this->_status_message = "Unknown"; // Default if no message provided
                }
            } else {
                // Other headers
                setHeader(key, value);
            }
        }
    }

    setBody(bodyPart, getHeaderValue("Content-Type").empty() ? "text/plain" : getHeaderValue("Content-Type"));
    // Update Content-Length based on the actual body size
    setHeader("Content-Length", intToString(this->_body.size()));
}
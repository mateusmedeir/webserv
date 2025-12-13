#include "../includes/WebservHeader.hpp"
#include "../includes/CookieHandler.hpp"

HttpResponse::HttpResponse(){
	this->_http_version = "HTTP/1.0";
	this->_status_code = 200;
	this->_status_message = "OK";
};

HttpResponse::~HttpResponse(){};

void HttpResponse::handleGet(const ServerBlock &serverBlock, const LocationBlock *location){
	if (!location->getReturn().empty()) return setResponseByStatus(302, location->getReturn(), "text/html");

	std::string path = location->getPath(serverBlock.getRoot().second);
	if (path.empty()) return setResponseByStatus(404);

	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file) return setResponseByStatus(404);

	std::ostringstream buffer;
	buffer << file.rdbuf();

	setResponseByStatus(200, buffer.str(), getMimeType(path));
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

void HttpResponse::handleDelete(const ServerBlock &serverBlock, const LocationBlock *location){
		std::string path = location->getPath(serverBlock.getRoot().second);

		if (std::remove(path.c_str()) == 0) {
			this->setStatus(200, "OK");
			this->setBody("<h1>File deleted successfully</h1>", "text/html");
		} else {
			this->setStatus(404, "Not Found");
			this->setBody("<h1>404 Not Found</h1>", "text/html");
		}
};

void HttpResponse::dispatchRequest(const HttpRequest &req, const ServerBlock &serverBlock) {
	if (this->_status_code != 200)
		return;
	
	const LocationBlock* locationPtr = serverBlock.getValidLocation(req.getUri(), req.getMethod());
    if (!locationPtr) {
        this->setErrorPage(404);
        return;
    }


	if(req.getMethod() == "GET")
		return handleGet(serverBlock, locationPtr);
	else if(req.getMethod() == "POST")
		return handlePost(req);
	else if(req.getMethod() == "DELETE")
		return handleDelete(serverBlock, locationPtr);
	else 
		this->setErrorPage(405);

	processCookies(req, *locationPtr);
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

void HttpResponse::setResponseByStatus(int statusCode, const std::string &bodyContent, const std::string &contentType) {
	if (this->_status_code >= 400) {
		setErrorPage(this->_status_code);
	} else if (statusCode == 302) {
		setStatus(302, "Found");
		setHeader("Location", bodyContent);
		setBody("<h1>302 Found</h1>", "text/html");
	} else {
		setStatus(statusCode, "OK");
		setBody(bodyContent, contentType);
	}
}

void HttpResponse::processCookies(const HttpRequest &req, const LocationBlock &location) {
	if (location.getCookiesEnabled()) {
		CookieHandler::handleCookie(*this, req);
	}
}

std::string HttpResponse::getHttpVersion() const {
	return _http_version;
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
#include "../includes/WebservHeader.hpp"

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
	}

	std::ostringstream buffer;
	buffer << file.rdbuf();
	this->setStatus(200, "OK");
	this->setBody(buffer.str(), "text/html"); // simplificado (todo: detectar mime-type)
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

    if (path[path.size() - 1] == '/') {
    path += "index";
    }
    if (path[0] != '/')
    path = "/" + path;

    std::cout << "Converted URI to path: " << "./www" + path + ".html" << std::endl;
    return "./www" + path + ".html";
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
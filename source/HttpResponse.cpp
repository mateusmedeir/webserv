#include "../includes/WebservHeader.hpp"

HttpResponse::HttpResponse(){};

HttpResponse::HttpResponse(HttpRequest const &req){
	this->http_version = "HTTP/1.0";
	this->status_code = 200;
	this->status_message = "OK";

	dispatchRequest(req);
};

HttpResponse::~HttpResponse(){};

void HttpResponse::handleGet(const HttpRequest &req) {
	std::string path = "./www" + req.getUri(); // root simplificada

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
		std::string path = "./www" + req.getUri();

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
	if(req.getMethod() == "GET")
		return handleGet(req);
	if(req.getMethod() == "POST")
		return handlePost(req);
	if(req.getMethod() == "DELETE")
		return handleDelete(req);

	this->setStatus(405, "Method not allowed");
	this->setHeader("Content-type", "text/html");
	this->setBody("<h1>405: Method not allowed</h1>","text/plain");
}

void		HttpResponse::setStatus(int code, const std::string &message){
	this->status_code = code;
	this->status_message = message;
};

void		HttpResponse::setHeader(const std::string &key, const std::string &value){
	this->headers[key] = value;
};

void		HttpResponse::setBody(const std::string &b, const std::string &contentType){
	body = b;
	this->headers["Content-Type"] = contentType;
	this->headers["Content-Length"] = intToString(body.size());
}

std::string	HttpResponse::toString() const{
	std::ostringstream response;

	// Status line
	response << http_version << " "
				<< status_code << " "
				<< status_message << "\r\n";

	// Headers
	for (std::map<std::string, std::string>::const_iterator it = headers.begin();
			it != headers.end(); ++it) {
		response << it->first << ": " << it->second << "\r\n";
	}

	// Linha em branco
	response << "\r\n";

	// Corpo
	response << body;

	return response.str();
}

std::string	HttpResponse::intToString(int n) const{
	std::ostringstream oss;
	oss << n;
	return oss.str();
}
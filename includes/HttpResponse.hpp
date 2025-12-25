#pragma once

# include "WebservHeader.hpp"

class Client;
class LocationBlock;
class ServerBlock;

class HttpResponse {
	private:
		std::string													_http_version;
		int 																_status_code;
		std::string 												_status_message;
		std::map<std::string, std::string>	_headers;
		std::string 												_body;
		bool																_execAutoIndex;
	public:
		bool																sended;

		HttpResponse();
		~HttpResponse();

	void generateAutoIndexHTML(const HttpRequest &req, const ServerBlock &serverBlock, const LocationBlock &location);
	void handleGet(const HttpRequest &req, const ServerBlock &serverBlock, const LocationBlock &location);
	void handlePost(const HttpRequest &req, const ServerBlock &serverBlock, const LocationBlock &location);
	void handleDelete(const HttpRequest &req, const ServerBlock &serverBlock, const LocationBlock &location);
	void dispatchRequest(Client *client, const ServerBlock &serverBlock, const LocationBlock &location);

	void setStatus(int code, const std::string &message);
	void setHeader(const std::string &key, const std::string &value);
	void setBody(const std::string &b, const std::string &contentType);
	void setErrorPage(int code, const ServerBlock *serverBlock = NULL);
	void setResponseByStatus(int statusCode, const ServerBlock *serverBlock = NULL, const std::string &bodyContent="", const std::string &contentType="text/html");
	void processCookies(const HttpRequest &req, const LocationBlock &location);
	void parseCgiOutput(const std::string& cgiRawOutput); // Adicionado

	std::string		toString() const;
	std::string		intToString(int n) const;

	std::string		getMimeType(const std::string &path) const;
	std::string		getStatusMessageForCode(int code) const;
	std::string 	getHttpVersion() const;
	void setExecAutoIndex(bool exec);
	
	std::string findBestLocationMatch(const std::string &uri, const ServerBlock &serverBlock, LocationBlock &location) const;

	bool 			getExecAutoIndex() const;
	
	int getStatusCode() const;
	std::string getStatusMessage() const;
	std::string getHeaderValue(const std::string &key) const;
};
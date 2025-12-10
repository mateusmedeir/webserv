# pragma once

# include "WebservHeader.hpp"

class LocationBlock;
class ServerBlock;

class HttpResponse {
	private:
		std::string													_http_version;
		int 																_status_code;
		std::string 												_status_message;
		std::map<std::string, std::string>	_headers;
		std::string 												_body;
	public:
		HttpResponse();
		~HttpResponse();

	void handleGet(const ServerBlock &serverBlock, const LocationBlock *location);
	void handlePost(const HttpRequest &req);
	void handleDelete(const ServerBlock &serverBlock, const LocationBlock *location);
	void dispatchRequest(const HttpRequest &req, const ServerBlock &serverBlock);
	bool dispatchRequestAsync(const HttpRequest &req, const ServerBlock &serverBlock, int clientFd);

	void setStatus(int code, const std::string &message);
	void setHeader(const std::string &key, const std::string &value);
	void setBody(const std::string &b, const std::string &contentType);
	void setErrorPage(int code);
	void processCookies(const HttpRequest &req, const LocationBlock &location);
	void processCgiResponse(const std::string &cgiOutput);
	
	std::string findBestLocationMatch(const std::string &uri, const ServerBlock &serverBlock, LocationBlock &location) const;

	std::string		toString() const;
	std::string		intToString(int n) const;
	bool					validatePath(const std::string &path) const;
	std::string		getMimeType(const std::string &path) const;
	std::string 	getHttpVersion() const;
	
	int getStatusCode() const;
	std::string getStatusMessage() const;
	std::string getHeaderValue(const std::string &key) const;
};
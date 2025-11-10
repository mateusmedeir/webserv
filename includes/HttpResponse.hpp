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

	void handleGet(const HttpRequest &req);
	void handlePost(const HttpRequest &req);
	void handleDelete(const HttpRequest &req);
	void dispatchRequest(const HttpRequest &req);
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
	std::string		uriToPath(const std::string &uri) const;
	std::string		getMimeType(const std::string &path) const;
	
	int getStatusCode() const;
	std::string getStatusMessage() const;
	std::string getHeaderValue(const std::string &key) const;
};
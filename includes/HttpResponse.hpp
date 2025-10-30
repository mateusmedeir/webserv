# pragma once

# include "WebservHeader.hpp"


class HttpResponse {
	private:
		std::string													_http_version;
		int 																_status_code;
		std::string 												_status_message;
		std::map<std::string, std::string>	_headers;
		std::string 												_body;
		std::vector<std::string>						_setCookieHeaders;
	public:
		HttpResponse();
		~HttpResponse();

	void handleGet(const HttpRequest &req);
	void handlePost(const HttpRequest &req);
	void handleDelete(const HttpRequest &req);
	void handleCgi(const HttpRequest &req, const std::string &scriptPath);
	void dispatchRequest(const HttpRequest &req);

	void setStatus(int code, const std::string &message);
	void setHeader(const std::string &key, const std::string &value);
	void setBody(const std::string &b, const std::string &contentType);
	void setErrorPage(int code);

	std::string		toString() const;
	std::string		intToString(int n) const;
	std::string		uriToPath(const std::string &uri) const;
	std::string		getMimeType(const std::string &path) const;
	
	//| CGI helpers
	bool isCgiRequest(const std::string &uri) const;
	std::string getCgiScriptPath(const std::string &uri) const;
	
	//| Cookies
	void setCookie(const std::string &name, const std::string &value, const std::string &path = "/", int maxAge = -1);
	void clearCookie(const std::string &name);
};
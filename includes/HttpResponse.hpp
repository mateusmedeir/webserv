# pragma once

# include "WebservHeader.hpp"


class HttpResponse {
	private:
		std::string http_version;
		int status_code;
		std::string status_message;
		std::map<std::string, std::string> headers;
		std::string body;
	public:
		HttpResponse();
		HttpResponse(HttpRequest const &req);
		~HttpResponse();

		void handleGet(const HttpRequest &req);
		void handlePost(const HttpRequest &req);
		void handleDelete(const HttpRequest &req);
		void dispatchRequest(const HttpRequest &req);

		void setStatus(int code, const std::string &message);
		void setHeader(const std::string &key, const std::string &value);
		void setBody(const std::string &b, const std::string &contentType);

		std::string		toString() const;
		std::string		intToString(int n) const;
};
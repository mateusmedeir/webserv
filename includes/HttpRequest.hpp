# pragma once

# include "WebservHeader.hpp"

class HttpRequest {
  private:
    std::string                         method;
    std::string                         uri;
    std::string                         version;
    std::map<std::string, std::string>  headers;
    std::string                         body;
    std::map<std::string, std::string>  cookies;

    
    public:
		HttpRequest();
		HttpRequest(const std::string &rawRequest);
		~HttpRequest();

    void parseRequestLine(const std::string &rawRequest);
    void parseHeaders(const std::string &rawRequest);
    void parseBody(const std::string &rawRequest);

    std::string getMethod() const;
    std::string getUri() const;
    std::map<std::string, std::string> getHeaders() const;
    std::string getHeaderValue(const std::string &key) const;
    std::string getBody() const;
    
    const std::map<std::string, std::string> &getCookies() const;
    std::string getCookie(const std::string &name) const;
    bool hasCookie(const std::string &name) const;
};
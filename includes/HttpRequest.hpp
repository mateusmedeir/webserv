# pragma once

# include "WebservHeader.hpp"

class HttpRequest {
  private:
    std::string                         method;
    std::string                         uri;
    std::string                         version;
    std::map<std::string, std::string>  headers;
    std::string                         body;
    std::string                         startBoundary;
    std::string                         endBoundary;
    std::string                         uploadFileName;
    bool                                isMultipart;
    bool                                isUpload;
    bool                                isCgi;

    
    public:
		HttpRequest();
		HttpRequest(const std::string &rawRequest);
		~HttpRequest();

    void parseRequestLine(const std::string &rawRequest);
    void parseHeaders(const std::string &rawRequest);
    void parseBody(const std::string &rawRequest, std::string onlyBody);

    std::string getMethod() const;
    std::string getUri() const;
    std::map<std::string, std::string> getHeaders() const;
    std::string getHeaderValue(const std::string &key) const;
    bool hasHeader(const std::string &key) const;
    void setIsCgi(bool val);
    std::string getBody() const;
    std::string getStartBoudary() const;
    std::string getEndBoudary() const;
    std::string getUploadFileName() const;
    bool getIsCgi() const;
    bool isUploadRequest();
};
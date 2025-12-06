#include "../includes/WebservHeader.hpp"

HttpRequest::HttpRequest(){}

HttpRequest::HttpRequest(const std::string &rawRequest) {
	parseRequestLine(rawRequest);
	parseHeaders(rawRequest);
	parseBody(rawRequest);
}

HttpRequest::~HttpRequest(){}

void HttpRequest::parseRequestLine(const std::string &rawRequest) {
	std::istringstream stream(rawRequest);
	std::string line;

	if (std::getline(stream, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		std::istringstream first_line(line);
		first_line >> this->method >> this->uri >> this->version;

		if (this->method.empty() || this->uri.empty() || this->version.empty())
			throw std::runtime_error("Request line malformada: campos ausentes");
	}
}

void HttpRequest::parseHeaders(const std::string &rawRequest) {
	std::istringstream stream(rawRequest);
	std::string line;
	bool pastFirstLine = false;

	while (std::getline(stream, line)) {
		if (!pastFirstLine) {
			pastFirstLine = true;
			continue;
		}

		if (line == "\r" || line == "") break;

		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		size_t sep = line.find(":");
		if (sep != std::string::npos) {
			std::string key = line.substr(0, sep);
			std::string value = line.substr(sep + 1);

			if (!value.empty() && value[0] == ' ')
				value.erase(0, 1);

			this->headers[key] = value;
		}
	}
}

void HttpRequest::parseBody(const std::string &rawRequest) {
	std::istringstream stream(rawRequest);
	std::string line;
	bool pastHeaders = false;

	while (std::getline(stream, line)) {
		if (!pastHeaders) {
			if (line == "\r" || line == "") {
				pastHeaders = true;
			}
			continue;
		}

		body += line + "\n";
	}
	this->body = body;
}

std::string HttpRequest::getMethod() const {
	return method;
}

std::string HttpRequest::getUri() const {
	return uri;
}

std::map<std::string, std::string> HttpRequest::getHeaders() const {
		return headers;
}

std::string HttpRequest::getHeaderValue(const std::string &key) const {
	std::string lowerKey = key;
	for (size_t i = 0; i < lowerKey.size(); ++i) {
		lowerKey[i] = std::tolower(lowerKey[i]);
	}
	
	for (std::map<std::string, std::string>::const_iterator it = headers.begin();
		 it != headers.end(); ++it) {
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

bool HttpRequest::hasHeader(const std::string &key) const {
	std::string lowerKey = key;
	for (size_t i = 0; i < lowerKey.size(); ++i) {
		lowerKey[i] = std::tolower(lowerKey[i]);
	}
	
	for (std::map<std::string, std::string>::const_iterator it = headers.begin();
		 it != headers.end(); ++it) {
		std::string lowerHeader = it->first;
		for (size_t i = 0; i < lowerHeader.size(); ++i) {
			lowerHeader[i] = std::tolower(lowerHeader[i]);
		}
		if (lowerHeader == lowerKey) {
			return true;
		}
	}
	return false;
}

std::string HttpRequest::getBody() const {
		return body;
}
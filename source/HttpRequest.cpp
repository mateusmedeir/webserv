#include "../includes/WebservHeader.hpp"

HttpRequest::HttpRequest(){}

// HttpRequest::HttpRequest(const std::string &rawRequest) {
// 	parseRequestLine(rawRequest);
// 	parseHeaders(rawRequest);
// 	parseBody(rawRequest, );
// }

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
			if (value.find("multipart/form-data") != std::string::npos) {
				this->isMultipart = true;
			}
		}
	}
}

std::string decodeChunkedBody(std::string& chunkedBody) {
	std::stringstream result;
	std::istringstream stream(chunkedBody);

	std::string line;
	while (std::getline(stream, line)) {
		// Convert the chunk size to an integer
		std::stringstream sizeStream(line);
		size_t chunkSize;
		sizeStream >> std::hex >> chunkSize;
		
		// If the chunk size is 0, it's the end of the message
		if (chunkSize == 0) {
			break;
		}
		
		// Read the chunk data
		std::vector<char> data(chunkSize);
		stream.read(data.data(), chunkSize);
		
		// Read and discard the newline after the chunk data
		stream.ignore(2, '\r');
		stream.ignore(2, '\n');
		
		// Append the chunk data to the result
		result.write(data.data(), chunkSize);
	}
	return result.str();
}

void HttpRequest::parseBody(const std::string &rawRequest, std::string onlyBody) {
	std::istringstream stream(rawRequest);
	std::string line;
	this->body = onlyBody;

	std::cout << "---------------PARSE BODY Raw Request-----------------" << std::endl;
	std::cout << rawRequest << std::endl;
	std::cout << "---------------PARSE BODY Only body-----------------" << std::endl;
	std::cout << onlyBody << std::endl;

	if (isMultipart) {
		std::cout << "IS MULTIPART FORM-DATA !!" << std::endl;
		this->isUpload = true;
		//get the boundary
		size_t boundaryStartPos = rawRequest.find("boundary=") + std::strlen("boundary=");
		size_t boundaryEndPos = rawRequest.find("\r\n", boundaryStartPos);
		this->startBoundary = rawRequest.substr(boundaryStartPos, (boundaryEndPos - boundaryStartPos));
		this->endBoundary = this->startBoundary + "--";
	}
	if (this->getHeaderValue("Transfer-Encoding") == "chunked")
		this->body = decodeChunkedBody(this->body);
	if (this->isUploadRequest()) {
		// Precisamos pegar o file name
		size_t filenameStartPos = this->body.find("filename=") + std::strlen("filename=");
		size_t filenameEndPos = this->body.find("\r\n", filenameStartPos);
		this->uploadFileName = this->body.substr(filenameStartPos, filenameEndPos - filenameStartPos);
	}
	std::cout << "---------------BODY UNCHUNKED-----------------" << std::endl;
	std::cout << this->getBody() << std::endl;

	// while (std::getline(stream, line)) {
	// 	if (!pastHeaders) {
	// 		if (line == "\r" || line == "") {
	// 			pastHeaders = true;
	// 		}
	// 		continue;
	// 	}

	// 	body += line + "\n";
	// }
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

std::string HttpRequest::getStartBoudary() const {
	return (this->startBoundary);
}

std::string HttpRequest::getEndBoudary() const {
	return (this->endBoundary);
}

std::string HttpRequest::getUploadFileName() const {
	return (this->uploadFileName);
}

bool HttpRequest::isUploadRequest() {
	return (this->isUpload);
}
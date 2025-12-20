#include "../includes/WebservHeader.hpp"
#include "../includes/RunTime.hpp"

Client::Client(int clientFd, ServerListen &serverListen) : EpollHandler(EPOLLIN | EPOLLOUT, clientFd, 10), _serverListen(serverListen) {
    this->_state = READING_HEADER;
    this->_rawRequest = "";
    this->request = HttpRequest();
    this->response = HttpResponse();
    this->_pendingResponse = "";
    this->_responseOffset = 0;
    this->cgiHandler = NULL;
}

Client::Client(const Client &src) : EpollHandler(src.getInterestedEvents(), src.getSocketFd(), src.getMaxTimeoutSecs()), _serverListen(src._serverListen) {
    *this = src;
}

Client &Client::operator=(const Client &src) {
    if (this != &src) {
        this->request = src.request;
        this->response = src.response;
        this->_state = src._state;
        this->_pendingResponse = src._pendingResponse;
        this->_responseOffset = src._responseOffset;
        this->cgiHandler = NULL; 
    }
    return (*this);
}

Client::~Client(void) {
    if (this->getSocketFd() != -1) {
        this->response.setErrorPage(504);
        sendResponse(this->response.toString());
        close(this->getSocketFd());
    }
    if (this->cgiHandler) {
        this->cgiHandler = NULL;
    }
}

void Client::handleEpollIn(void) {
    if (this->_state == WAITING_CGI) {
        return;
    }

    char buffer[MAX_BUFFER_SIZE] = {0};
    int count = 0;
    if ((count = read(this->getSocketFd(), buffer, sizeof(buffer))) > 0) {
        this->concatenateRequestData(std::string(buffer, count));
        if (this->isRequestComplete()) {
            Logger::debug("Client: Request Header:\n" + this->getRawRequest());
            ServerBlock serverBlock = this->_serverListen.getServerBlock();
            const LocationBlock* locationPtr = serverBlock.getValidLocation(this->request.getUri(), this->request.getMethod());
            if (!locationPtr) {
                this->response.setErrorPage(404, &serverBlock);
            } else {
                this->request.setIsCgi(CgiHandler::isCgiScript(this->request.getUri(), *locationPtr));
                if (!validatingUriWithLocation(serverBlock, const_cast<LocationBlock&>(*locationPtr))) {
                    Logger::error("Erro nas validacoes dos metodos da request...");
                    return ;
                }
                this->response.dispatchRequest(this, this->_serverListen.getServerBlock(), *locationPtr);
            }
            
            if (this->_state != WAITING_CGI) { 
                std::string responseStr = this->response.toString();
                Logger::info(toString());
                if (!sendResponse(responseStr)) {
                    return;
                }
            }
        }
    } else if (count == 0) {
        EpollInstance::manipInterestList(EPOLL_CTL_DEL, this);
    }
}

void Client::handleEpollOut(void) {
    if (this->_state == WAITING_CGI) {
        if (this->cgiHandler && this->cgiHandler->isFinished()) {
            Logger::debug("Client: CGI handler finished, processing output.");
            std::string cgiOutput = this->cgiHandler->getCgiOutput();
            this->response.parseCgiOutput(cgiOutput);
            this->_state = COMPLETE;

            EpollInstance::manipInterestList(EPOLL_CTL_DEL, this->cgiHandler);
            this->cgiHandler = NULL;

        } else if (this->cgiHandler) {
            // CGI still running, do nothing and wait.
            return;
        } else {
            Logger::error("Client: In WAITING_CGI state but cgiHandler is NULL. Sending 500.");
            ServerBlock serverBlock = this->_serverListen.getServerBlock();
            this->response.setErrorPage(500, &serverBlock);
            this->_state = COMPLETE;
        }
    }

    if (!this->_pendingResponse.empty()) {
        if (!sendResponse(this->_pendingResponse)) {
            // sendResponse returning false means the client was closed/deleted.
            return;
        }
        if (!this->_pendingResponse.empty()) {
            // Partial send, wait for next EPOLLOUT.
            return;
        }
    }

    if (this->isRequestComplete() && this->_pendingResponse.empty()) {
        std::string responseStr = this->response.toString();
        Logger::info(toString());
        if (!sendResponse(responseStr)) {
            // Client was closed/deleted.
            return;
        }
        if (this->_pendingResponse.empty()) {
            EpollInstance::manipInterestList(EPOLL_CTL_DEL, this);
        }
    }
}

bool Client::sendResponse(const std::string &responseStr) {
    if (this->_pendingResponse.empty()) {
        this->_pendingResponse = responseStr;
        this->_responseOffset = 0;
    }
    
    const char *data = this->_pendingResponse.c_str() + this->_responseOffset;
    size_t remaining = this->_pendingResponse.size() - this->_responseOffset;
    
    // send() com flags MSG_NOSIGNAL para evitar SIGPIPE
    ssize_t sent = send(this->getSocketFd(), data, remaining, MSG_NOSIGNAL);
    
    if (sent < 0) {
        EpollInstance::manipInterestList(EPOLL_CTL_DEL, this);
        return true;
    } else if (sent == 0) {
        Logger::debug("Client: Connection closed by peer during send, closing client.");
        EpollInstance::manipInterestList(EPOLL_CTL_DEL, this);
        return false;
    } else {
        // send() enviou alguns bytes (pode ser parcial)
        this->_responseOffset += sent;

        if (this->_responseOffset < this->_pendingResponse.size()) {
            uint32_t events = this->getInterestedEvents();
            events |= EPOLLOUT;
            this->setInterestedEvents(events);
            EpollInstance::manipInterestList(EPOLL_CTL_MOD, this);
        }
        
        return true;
    }
}

void Client::concatenateRequestData(std::string data) {
    if (this->_state == COMPLETE || this->_state == WAITING_CGI) {
        return;
    }
    this->_rawRequest.append(data);

    if (this->_state == READING_HEADER && this->_rawRequest.find("\r\n\r\n") != std::string::npos) {
        this->request.parseRequestLine(this->_rawRequest);
        this->request.parseHeaders(this->_rawRequest);

        ServerBlock serverBlockRef = this->_serverListen.getServerBlock();
        if (!serverBlockRef.isUriValid(this->request.getUri())) {
            this->response.setErrorPage(404, &serverBlockRef);
            this->setState(COMPLETE);
            return;
        }
        if (
            !serverBlockRef.getValidLocation(this->request.getUri(), this->request.getMethod())
        ) {
            this->response.setErrorPage(405, &serverBlockRef);
            this->setState(COMPLETE);
            return;
        }
        this->setState(READING_BODY);
    }

    if (this->_state == READING_BODY) {
        std::string contentLengthStr = this->request.getHeaderValue("Content-Length");

        if (!contentLengthStr.empty()) { 
            int contentLength = std::atoi(contentLengthStr.c_str());
            size_t bodyStartPos = this->_rawRequest.find("\r\n\r\n") + 4;
            size_t bodyLength = this->_rawRequest.size() - bodyStartPos;
            std::string onlyBody = this->_rawRequest.substr(bodyStartPos, bodyLength);
            
            if (bodyLength >= static_cast<size_t>(contentLength)) {
                this->request.parseBody(this->_rawRequest, onlyBody);
                this->setState(COMPLETE);
            }
        } else if (this->_rawRequest.find("0\r\n\r\n") != std::string::npos) {
            //caso tenha outro encoding (chunked)
            // caso entre aqui, o body da request ja ta todo pronto.
            size_t bodyStartPos = this->_rawRequest.find("\r\n\r\n") + 4;
            size_t bodyEndPos = this->_rawRequest.find("0\r\n\r\n") + 4;
            std::string onlyBody = this->_rawRequest.substr(bodyStartPos, bodyEndPos);
            this->request.parseBody(this->_rawRequest, onlyBody);
            this->setState(COMPLETE);
        } else {
            this->setState(COMPLETE);
        }
    }
}

bool	isDirectory(const std::string& path) {
	struct stat path_stat;
	if (stat(path.c_str(), &path_stat) != 0) {
		// Erro ao acessar as informacoes do arquivo.
		return false;
	}
	return S_ISDIR(path_stat.st_mode);
}

bool Client::validateMethodAllowed(LocationBlock &location) {
    if (!location.checkHttpMethodInLocation(this->request.getMethod())) {
        Logger::debug("Metodo nao permitido na location...");
        ServerBlock serverBlock = this->_serverListen.getServerBlock();
        this->response.setResponseByStatus(405, &serverBlock);
        return false;
    }
    return true;
}

bool Client::validatingUriWithLocation(ServerBlock &serverBlock, LocationBlock &location) {
    if (!validateMethodAllowed(location))
        return false;

    const std::string &method = this->request.getMethod();

    if (method == "GET")
        return validateGet(serverBlock, location);
    else if (method == "POST")
        return validatePost(serverBlock, location);
    else if (method == "DELETE")
        return validateDelete(serverBlock, location);
    else {
        Logger::debug("Metodo HTTP nao suportado...");
        this->response.setResponseByStatus(405, &serverBlock);
        return false;
    }
}

bool Client::validateGet(ServerBlock &serverBlock, LocationBlock &location) {
    std::string path = serverBlock.getRoot().second + this->request.getUri();
    path = extractUriWithoutQuery(path);
    Logger::debug("String contendo alias + uri para o GET: " + path);

    if (access(path.c_str(), R_OK) != 0) {
        Logger::debug("Acesso ao recurso " + path + " negado.");
        this->response.setResponseByStatus(403, &serverBlock);
        return false;
    }

    if (!path.empty() && path[path.size() - 1] == '/') {
        std::vector<std::string> indexes = location.getIndex();
        for (size_t i = 0; i < indexes.size(); i++) {
            if (access((path + indexes[i]).c_str(), R_OK) == 0)
                return true;
        }

        if (!location.getAutoIndex()) {
            Logger::debug("Autoindex desabilitado e nenhum index encontrado.");
            this->response.setResponseByStatus(403, &serverBlock);
            return false;
        }

        Logger::debug("Autoindex habilitado.");
        this->response.setExecAutoIndex(true);
        return true;
    }

    if (isDirectory(path)) {
        Logger::debug("Acesso ao diretorio " + path + " negado.");
        this->response.setResponseByStatus(403, &serverBlock);
        return false;
    }

    return true;
}

bool Client::validatePost(ServerBlock &serverBlock, LocationBlock &location) {
    std::string path = serverBlock.getRoot().second + this->request.getUri();
    path = extractUriWithoutQuery(path);
    std::string uri = this->request.getUri();

    Logger::debug("client uri: " + uri);
    Logger::debug("location uri: " + location.getUri());

    if (!this->request.getIsCgi() && (uri.empty() || uri != location.getUri())) {
        this->response.setResponseByStatus(404, &serverBlock);
        return (false);
    } else if (this->request.getIsCgi()) {
        Logger::debug("Validating POST for CGI script at path: " + path);
        if (access(path.c_str(), F_OK) != 0) {
            this->response.setResponseByStatus(404, &serverBlock);
            return (false);
        } else if (access(path.c_str(), R_OK) != 0) {
            this->response.setResponseByStatus(403, &serverBlock);
            return (false);
        } else {
            return (true);
        }
    }

    (void)serverBlock; // Unused parameter

    // if (uri.empty() ||
    //     uri[uri.size() - 1] == '/') {
    //     this->response.setResponseByStatus(400, &serverBlock);
    //     return false;
    // }

    if (this->_serverListen.getServerBlock().getMaxBodySize().second <
        this->request.getBody().size()) {
        this->response.setResponseByStatus(413, &serverBlock);
        return false;
    }

    if (!location.getCanUpload() || location.getUploadPath().empty()) {
        Logger::debug("Upload nao permitido nesta location...");
        this->response.setResponseByStatus(403, &serverBlock);
        return false;
    }

    std::string uploadDir = location.getUploadPath();
    if (access(uploadDir.c_str(), R_OK | W_OK) != 0) {
        Logger::debug("Acesso ao diretorio de upload negado...");
        this->response.setResponseByStatus(403, &serverBlock);
        return false;
    }

    return true;
}

bool Client::validateDelete(ServerBlock &serverBlock, LocationBlock &location) {
	std::string locationUploadDir = location.getUploadPath();
	if (locationUploadDir.empty()) return (this->response.setResponseByStatus(404, &serverBlock), false);

	std::string uri = this->request.getUri();
	if (uri[uri.size() - 1] == '/') return (this->response.setResponseByStatus(404, &serverBlock), false);

	size_t	filePos = uri.rfind('/');
	std::string fileName = uri.substr(filePos);
    std::string newUri;
    if ((filePos + 1) <= uri.size()) {
        newUri = uri.substr(0, (filePos + 1));
    }
    else {
        newUri = uri.substr(0, filePos);
    }

    // std::string uri = this->request.getUri();
    // uri = extractUriWithoutQuery(uri);

    Logger::debug("client uri: " + newUri);
    Logger::debug("location uri: " + location.getUri());

    if (newUri.empty() || newUri != location.getUri()) {
        this->response.setResponseByStatus(404, &serverBlock);
        return (false);
    }

    (void)serverBlock; // Unused parameter

    // if (uri.empty() ||
    //     uri[uri.size() - 1] == '/') {
    //     this->response.setResponseByStatus(403, &serverBlock, "Forbidden", "<h1>Forbidden</h1>");
    //     return false;
    // }

    std::string base;
    if (this->request.getIsCgi()) {
        base = serverBlock.getRoot().second;
    } else {
        base = location.getUploadPath();
    }
    std::string fullPath = "";

    Logger::debug("Base path for DELETE: " + base);
    if (base.empty())
        return false;

    // size_t	filePos = uri.rfind('/');
	// std::string fileName = uri.substr(filePos);

    if (base[base.size() - 1] == '/')
		fullPath = base + fileName;
	else
		fullPath = base + "/" + fileName;

    Logger::debug("Filename: " + fileName);

    Logger::debug("Full path for DELETE: " + fullPath);
    if (access(fullPath.c_str(), R_OK | W_OK) != 0) {
        this->response.setResponseByStatus(403, &serverBlock);
        return false;
    }

    return true;
}

bool Client::isRequestComplete(void) {
    return (this->_state == COMPLETE);
}

int Client::getState(void) const {
    return (this->_state);
}

std::string &Client::getRawRequest(void) {
    return (this->_rawRequest);
}

HttpRequest &Client::getRequest(void) {
    return (this->request);
}

HttpResponse &Client::getResponse(void) {
    return (this->response);
}

void Client::setState(int state) {
    this->_state = state;
}

static std::string ipv4ToStr(unsigned int ip)
{
    std::ostringstream ss;

    ss << ((ip >> 24) & 0xFF) << "."
       << ((ip >> 16) & 0xFF) << "."
       << ((ip >> 8)  & 0xFF) << "."
       << (ip & 0xFF);

    return ss.str();
}

// std::string toString(void) const;
std::string Client::toString(void) const {
    std::ostringstream result;

    // Data/hora/dia/mes/ano "Isso ja tem no logger"
    // +
    // [tipo] "Isso ja tem no logger"
    // +
    // Mensagem passada para o Logger::metodo(mensagem);

    // IP do socket onde foi feito a request
    // +
    // Metodo http
    // +
    // URI desejada
    // +
    // Protocolo usado
    // +
    // Status da response
    result << ipv4ToStr(this->_serverListen.getHost()) << ": [" //Precisamos converter esse int para o IP que o socket ouve
            << this->request.getMethod() << "] "
            << this->request.getUri() << " "
            << this->response.getHttpVersion() << " "
            << this->response.getStatusCode();
    
            return(result.str());
}
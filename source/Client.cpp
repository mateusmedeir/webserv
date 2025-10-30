#include "../includes/WebservHeader.hpp"
#include "../includes/RunTime.hpp"

Client::Client(int clientFd, ServerListen &serverListen) : EpollHandler(clientFd, EPOLLIN), _serverListen(serverListen), _cgi(NULL) {
    std::cout << "Client got created..." << std::endl;
    this->_state = READING_HEADER;
    this->_rawRequest = "";
    this->request = HttpRequest();
    this->response = HttpResponse();
    this->_lastActivity = time(NULL);  // Inicializar timestamp
}

Client::Client(const Client &src) : EpollHandler(src.getSocketFd(), src.getInterestedEvents()), _serverListen(src._serverListen) {
    *this = src;
}

Client &Client::operator=(const Client &src) {
    if (this != &src) {
        this->request = src.request;
        this->response = src.response;
        this->_state = src._state;
        this->_lastActivity = src._lastActivity;
        this->_cgi = src._cgi;
        this->_rawRequest = src._rawRequest;
    }
    return (*this);
}

Client::~Client(void) {
    // Limpar CGI se existir
    if (_cgi != NULL) {
        delete _cgi;
        _cgi = NULL;
    }
}

void Client::handleEpollIn(void) {
    char buffer[4096] = {0};
    int count = 0;

    if ((count = read(this->getSocketFd(), buffer, sizeof(buffer))) > 0) {
        this->updateActivity();  // Atualizar timestamp de atividade
        this->concatenateRequestData(std::string(buffer, count));
        if (this->isRequestComplete()) {
            std::cout << "================== REQUEST COMPLETE =================" << std::endl;
            std::cout << this->request.getMethod() << std::endl;
            std::cout << this->request.getUri() << std::endl;
            std::map<std::string, std::string> headers = this->request.getHeaders();
            for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++) {
                std::cout << it->first << ": " << it->second << std::endl;
            }
            std::cout << "Body: " << this->request.getBody() << std::endl;
            std::cout << "=====================================================" << std::endl;
            
            // Processar requisição
            this->response.dispatchRequest(this->request);
            
            // ✅ NOVO: Verificar se é requisição CGI
            if (this->response.isCgiRequest(this->request.getUri())) {
                std::string scriptPath = this->response.getCgiScriptPath(this->request.getUri());
                
                // Verificar se o script existe antes de iniciar CGI
                std::ifstream scriptFile(scriptPath.c_str());
                if (scriptFile.good()) {
                    scriptFile.close();
                    
                    // Iniciar execução assíncrona do CGI
                    this->startCgiExecution(scriptPath);
                    
                    // Estado mudou para EXECUTING_CGI
                    // O Client vai aguardar eventos do CGI via Epoll
                    std::cout << "[Client] CGI started asynchronously, waiting for completion..." << std::endl;
                    return;  // Não envia resposta ainda!
                }
                // Se script não existe, handleCgi já configurou erro 404
            }
            
            // Enviar resposta (se não for CGI ou se CGI deu erro)
            std::string responseStr = this->response.toString();
            std::cout << "=================== RESPONSE SEND ===================" << std::endl;
            std::cout << responseStr << std::endl;
            std::cout << "=====================================================" << std::endl;
            send(this->getSocketFd(), responseStr.c_str(), responseStr.size(), 0);
            RunTime::deleteClient(this->getSocketFd());
        }
    } else if (count == 0) {
        std::cout << "Client closed the connection." << std::endl;
        std::cout << this->getRawRequest() << std::endl;
        RunTime::deleteClient(this->getSocketFd());
    }
}

void Client::concatenateRequestData(std::string data) {
    if (this->_state == COMPLETE) {
        return;
    }

    this->_rawRequest.append(data);

    if (this->_state == READING_HEADER && this->_rawRequest.find("\r\n\r\n") != std::string::npos) {
        this->request.parseRequestLine(this->_rawRequest);
        this->request.parseHeaders(this->_rawRequest);

        if (!this->_serverListen.getServerBlock().isUriValid(this->request.getUri())) {
            this->response.setErrorPage(404);
            this->setState(COMPLETE);
            return;
        }
        if (
            !this->_serverListen.getServerBlock()
            .isLocationValid(
                this->request.getUri(),
                this->request.getMethod()
            )
        ) {
            this->response.setErrorPage(405);
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
            
            if (bodyLength >= static_cast<size_t>(contentLength)) {
                this->request.parseBody(this->_rawRequest);
                this->setState(COMPLETE);
            }
        } else {
            this->setState(COMPLETE);
        }
    }
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

/**
 * @brief Inicia execução assíncrona de CGI
 */
void Client::startCgiExecution(const std::string &scriptPath) {
    std::cout << "[Client] Starting async CGI execution for: " << scriptPath << std::endl;
    
    try {
        // Criar objeto CGI
        _cgi = new Cgi(scriptPath, this->request, this->response);
        
        // Executar (fork + pipes)
        _cgi->execute();
        
        // Adicionar FD do CGI ao Epoll
        // Dependendo do estado inicial (WRITING ou READING), adiciona o FD correto
        RunTime::getEpoll().manipInterestList(EPOLL_CTL_ADD, _cgi);
        
        // Mudar estado do client para aguardando CGI
        this->_state = EXECUTING_CGI;
        
        std::cout << "[Client] CGI added to Epoll, FD: " << _cgi->getSocketFd() << std::endl;
        
    } catch (const std::exception &e) {
        std::cerr << "[Client] Error starting CGI: " << e.what() << std::endl;
        this->response.setErrorPage(500);
        this->_state = COMPLETE;
        
        if (_cgi != NULL) {
            delete _cgi;
            _cgi = NULL;
        }
    }
}

/**
 * @brief Verifica se CGI completou e processa resposta
 */
void Client::checkCgiCompletion(void) {
    if (_cgi == NULL || !_cgi->isDone()) {
        return;
    }
    
    std::cout << "[Client] CGI completed!" << std::endl;
    
    // Verificar timeout
    if (_cgi->isTimedOut(5)) {
        std::cerr << "[Client] CGI timeout!" << std::endl;
        _cgi->killProcess();
        this->response.setErrorPage(504);
    }
    
    // Remover CGI do Epoll
    try {
        RunTime::getEpoll().manipInterestList(EPOLL_CTL_DEL, _cgi);
    } catch (...) {
        // Pode já ter sido removido
    }
    
    // Limpar CGI
    delete _cgi;
    _cgi = NULL;
    
    // Marcar como completo
    this->_state = COMPLETE;
}

/**
 * @brief Verifica se client tem CGI em execução
 */
bool Client::hasCgi(void) const {
    return (_cgi != NULL);
}

/**
 * @brief Retorna ponteiro para CGI (se houver)
 */
Cgi* Client::getCgi(void) const {
    return _cgi;
}

/**
 * @brief Verifica se o cliente excedeu o timeout de inatividade
 * @param timeoutSeconds Tempo máximo de inatividade em segundos
 * @return true se o cliente está inativo há mais tempo que o timeout
 */
bool Client::isTimedOut(int timeoutSeconds) const {
    time_t now = time(NULL);
    return (now - _lastActivity) > timeoutSeconds;
}

/**
 * @brief Atualiza o timestamp de última atividade
 */
void Client::updateActivity(void) {
    _lastActivity = time(NULL);
}
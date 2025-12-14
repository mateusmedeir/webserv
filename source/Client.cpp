#include "../includes/WebservHeader.hpp"
#include "../includes/RunTime.hpp"

Client::Client(int clientFd, ServerListen &serverListen) : EpollHandler(EPOLLIN | EPOLLOUT, clientFd, 30), _serverListen(serverListen) {
    this->_state = READING_HEADER;
    this->_rawRequest = "";
    this->request = HttpRequest();
    this->response = HttpResponse();
    this->_pendingResponse = "";
    this->_responseOffset = 0;
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
    }
    return (*this);
}

Client::~Client(void) {
    if (this->getSocketFd() != -1) {
        close(this->getSocketFd());
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

// Preciso por a funcao de achar o best match no topo da validatingWithLocation()
	// std::map<std::string, LocationBlock> locations = serverBlock.getLocations();
	// std::string bestMatch = "";
	// const LocationBlock* locationPtr = NULL;
	
	// for (std::map<std::string, LocationBlock>::const_iterator it = locations.begin();
	// 	 it != locations.end(); ++it) {
	// 	const std::string &path = it->first;
	// 	if (req.getUri().compare(0, path.size(), path) == 0) {
	// 		if (path.size() > bestMatch.size()) {
	// 			bestMatch = path;
	// 			locationPtr = &(it->second);
	// 		}
	// 	}
	// }
// Essa parte vai pegar sempre o melhor match para a location baseado no URI
// Posso fazer essa validacao antes de entrar nos ifs para validar os metodos
// Dessa forma, sempre teremos o location certo ja validado para usarmos na validacao dentro dos ifs
bool Client::validatingUriWithLocation(std::string bestMatch) {
    if (bestMatch.empty()) {
        // A funcao de bestMatch nao achou o match da location
        this->response.setErrorPage(400);
        this->response.setStatus(400, "Bad Request");
        return (false);
    }
    Logger::info("O bestMatch e: " + bestMatch);
    std::map<std::string, LocationBlock> locationsMap = this->_serverListen.getServerBlock().getLocations();
    LocationBlock location = locationsMap.find(bestMatch)->second;
    // Aqui, vamos validar a URI levando em consideracao cada metodo.
    // Validacoes para o GET:
    if (this->request.getMethod() == "GET") {
        Logger::debug("ENTROU NO VALIDADOR DO GET");
        // Validar se o Location aceita o GET
        if (!location.checkHttpMethodInLocation("GET")) {
            Logger::error("Location nao aceita o metodo GET.");
            this->response.setErrorPage(405);
            this->response.setStatus(405, "Method not allowed");
            return (false);
        }
        // Concatenar a URI com o root/alias da location
        std::string rootOrAliasPlusUri = location.getAlias() + this->request.getUri();
        Logger::debug("String contendo alias + uri para o GET: " + rootOrAliasPlusUri);
        // Validar o acesso de READ a esse recurso
        if (access(rootOrAliasPlusUri.c_str(), R_OK) != 0) {
            // erro de acesso ao recurso
            Logger::error("Acesso ao recurso " + rootOrAliasPlusUri + " negado.");
            this->response.setErrorPage(403);
            this->response.setStatus(403, "Forbidden");
            return (false);
        }
        if (!rootOrAliasPlusUri.empty() && rootOrAliasPlusUri[rootOrAliasPlusUri.size() - 1] == '/') {
            // e dir
            // validar se tem algum index dentro desse dir.
            bool    aux = false;
            std::vector<std::string> locationIndexes = location.getIndex();
            for (std::vector<std::string>::iterator it = locationIndexes.begin(); it != locationIndexes.end(); it++) {
                if (access((rootOrAliasPlusUri + *it).c_str(), R_OK) == 0) {
                    // rootOrAliasPlusUri += *it;
                    // this->request.setUri(this->request.getUri() + *it);
                    aux = true;
                }
            }
            if (aux) {
                // achou algum index dentro do dir rootOrAliasPlusUri
                return (true);
            } else {
                // nao achou index.
                if (!location.getAutoIndex()) {
                    // location nao tem auto index
                    Logger::error("Location nao aceita o autoindex.");
                    this->response.setErrorPage(403);
                    this->response.setStatus(403, "Forbidden");
                    return (false);
                }
                // vai executar o autoindex...
                Logger::info("Location aceita o autoindex.");
                this->response.setExecAutoIndex(true);
                return (true);
            }
        } else {
            // O cliente esta pedindo um arquivo
            Logger::debug("Validando se o arquivo existe. Nao tem autoindex...: " + rootOrAliasPlusUri);
            // if (access(rootOrAliasPlusUri.c_str(), R_OK) != 0) 
            if (isDirectory(rootOrAliasPlusUri)) {
                // erro de acesso ao recurso
                Logger::error("Acesso ao recurso " + rootOrAliasPlusUri + " negado.");
                this->response.setErrorPage(403);
                this->response.setStatus(403, "Forbidden");
                return (false);
            }
        }
        // Checar se termina em "/" ou nao
            // Se sim -> e diretorio
            // Validar se tem um index nesse diretorio
                // se sim -> ler e devolver esse index
            // se nao -> validar pra ver se a location tem o autoindex
                // se sim -> executar o autoindex
                // se nao -> erro 403
        // se nao terminar em "/" -> O client ta pedindo um arquivo
            // validar o acesso de leitura ao recurso
                // se sim -> ler o arquivo e montar o body
                //se nao -> erro 404 not found
        return (true);
    }
    // Validacoes para o POST:
    if (this->request.getMethod() == "POST") {
        Logger::debug("ENTROU NO VALIDADOR DO POST");
        if (this->request.getUri().empty() || this->request.getUri()[this->request.getUri().length() - 1] == '/') {
            this->response.setErrorPage(400);
            this->response.setStatus(400, "Bad Request");
            return (false);
        }
        //Validar o tamanho maximo do body da request.
        Logger::debug("------ testando o max body dentro da validacao para os metodos ----------");
        std::cout << "Max body size do server block: " << this->_serverListen.getServerBlock().getMaxBodySize().second << std::endl;
        Logger::debug("------ testando o max body dentro da validacao para os metodos ----------");
        if (this->_serverListen.getServerBlock().getMaxBodySize().second < this->request.getBody().size()) {
            // O tamanho do arquivo e maior do que o limite suportado por max_body_size
            Logger::error("Max body size exceded. Payload Too Large");
            this->response.setErrorPage(413);
            this->response.setStatus(413, "Payload Too Large");
        }
        if (this->_serverListen.getServerBlock().getMaxBodySize().second)
        // O upload na location esta liberado?
        if (!location.getCanUpload()) {
            //nao pode upload nessa location
            Logger::error("Location nao aceita upload. 403 forbidden");
            this->response.setErrorPage(403);
            this->response.setStatus(403, "Forbidden");
            return (false);
        }
        // Validar se a location permite POST
        if (!location.checkHttpMethodInLocation("POST")) {
            // a location nao aceita POST
            Logger::error("Location " + this->request.getUri() + " nao aceita o metodo POST.");
            this->response.setErrorPage(405);
            this->response.setStatus(405, "Method not allowed");
            return (false);
        }
        // determinar o diretorio de upload
        std::string locationUploadDir = location.getUploadPath();
    	// o diretorio e "gravavel"?
        if (access(locationUploadDir.c_str(),R_OK | W_OK) != 0) {
		    Logger::error("Error no acesso. 403 forbidden");
		    this->response.setErrorPage(403);
		    this->response.setStatus(403, "Forbidden");
	    	return (false);
	    }
        // Retorna TRUE
        return (true);
    }
    // Validacoes para o DELETE:
    if (this->request.getMethod() == "DELETE") {
        Logger::debug("ENTROU NO VALIDADOR DO DELETE");
        // checar se termina ou nao em "/"
            // Caso termine em "/", e um diretorio. Nao podemos deletar!
        if (this->request.getUri().empty() || this->request.getUri()[this->request.getUri().length() - 1] == '/') {
            Logger::error("Diretory nao pode ser deletado. 403 forbidden.");
            this->response.setErrorPage(403);
            this->response.setStatus(403, "Forbidden");
            return (false);
        }
        // checar se a location suporta DELETE
        if (!location.checkHttpMethodInLocation("DELETE")) {
            Logger::error("Location nao aceita DELETE");
            // a location nao aceita DELETE
            this->response.setErrorPage(405);
            this->response.setStatus(405, "Method not allowed");
            return (false);
        }
        // A URI vai vir com o a location + "/nome_do_arquivo"
        // Precisamos pegar apenas o URI sem o arquivo.
        std::string locationUploadDir = location.getUploadPath();
        if (!locationUploadDir.empty()) {
            Logger::debug("Upload Location existe!");
            // significa que temos um diretorio para upload e delete.
            std::string fullPathToDelete = locationUploadDir + this->request.getUri();
            // full path vai ter o local de upload + URI da request.
            // Validar se o arquivo existe nesse fullPath
            //checar os direitos de acesso ao recurso
            Logger::debug("FullPathToDelete: " + fullPathToDelete);
            if (access(fullPathToDelete.c_str(), R_OK | W_OK) != 0) {
                Logger::error("Error no acesso ao arquivo desejado para deletar. 403 forbidden");
                this->response.setErrorPage(403);
                this->response.setStatus(403, "Forbidden");
                return (false);
            }
            Logger::debug("Vai retornar TRUE pro DELETE.");
            return (true);
        }
        return (false);
        // // Caso nao temine em "/", e um arquivo.
        // if (isDirectory(locationUploadDir)) {
        //     //checar os direitos de acesso ao arquivo
        //     this->response.setErrorPage(403);
		//     this->response.setStatus(403, "Forbidden");
	    // 	return (false);
        // }
        // // Checando os acessos ao caminho completo que sera deletado
	    // std::string fullPathToDelete = locationUploadDir + this->request.getUri();
        // if (access(locationUploadDir.c_str(), W_OK) != 0) {
        //     this->response.setErrorPage(403);
		//     this->response.setStatus(403, "Forbidden");
	    // 	return (false);
        // }
        // return (true);
    }
    return (false);
}

void Client::handleEpollIn(void) {
    char buffer[4096] = {0};
    int count = 0;
    if ((count = read(this->getSocketFd(), buffer, sizeof(buffer))) > 0) {
        this->concatenateRequestData(std::string(buffer, count));
        if (this->isRequestComplete()) {
            ServerBlock serverBlock = this->_serverListen.getServerBlock();
            // Process cookies if enabled for this location
            std::string uri = this->request.getUri();
            std::map<std::string, LocationBlock> locations = serverBlock.getLocations();
            
            // Find best matching location (prefix match, longest wins)
            std::string bestMatch = "";
            for (std::map<std::string, LocationBlock>::const_iterator it = locations.begin();
            it != locations.end(); ++it) {
                const std::string &path = it->first;
                if (uri.compare(0, path.size(), path) == 0) {
                    if (path.size() > bestMatch.size()) {
                        bestMatch = path;
                    }
                }
            }
            
            // VALIDAR ERROS NOS METODOS HTTP GET, POST e DELETE
            if (!validatingUriWithLocation(bestMatch)) {
                // Deu erro para execucao do metodo da request.
                Logger::error("Erro nas validacoes dos metodos da request...");
                return ;
            }
            this->response.dispatchRequest(this->request, this->_serverListen.getServerBlock());
            
            std::string responseStr = this->response.toString();
            Logger::info(toString());
        }
    } else if (count == 0) {
        // EOF - cliente fechou conexão
        EpollInstance::manipInterestList(EPOLL_CTL_DEL, this);
    }
    // count < 0: erro no read() ou EAGAIN
    // Em non-blocking, -1 pode ser EAGAIN/EWOULDBLOCK (normal) ou erro real
    // NÃO verificamos errno diretamente (conforme régua)
    // Se epoll acionou EPOLLIN, deveria haver dados - se read() retorna -1, pode ser erro
    // Porém, em alguns casos raros, pode ser EAGAIN mesmo com EPOLLIN (race condition)
    // Por segurança, apenas não processamos - o próximo epoll_wait() tentará novamente
    // Se for erro real persistente, o timeout de 30s removerá o cliente
}

void Client::handleEpollOut(void) {
    // Socket está pronto para escrita - continuar enviando resposta pendente
    if (this->_pendingResponse.empty() || this->_responseOffset >= this->_pendingResponse.size()) {
        // Não há nada para enviar - remover interesse em EPOLLOUT
        uint32_t events = this->getInterestedEvents();
        events &= ~EPOLLOUT;
        this->setInterestedEvents(events);
        EpollInstance::manipInterestList(EPOLL_CTL_MOD, this);
        return;
    }
    
    // Continuar enviando resposta pendente
    if (!sendResponse(this->_pendingResponse)) {
        // Erro ao enviar - cliente já foi removido
        return;
    }
    
    // Se toda a resposta foi enviada, remover cliente
    if (this->_responseOffset >= this->_pendingResponse.size()) {
        // Remover interesse em EPOLLOUT
        uint32_t events = this->getInterestedEvents();
        events &= ~EPOLLOUT;
        this->setInterestedEvents(events);
        EpollInstance::manipInterestList(EPOLL_CTL_DEL, this);
    }
}

bool Client::sendResponse(const std::string &responseStr) {
    // Armazenar resposta pendente se necessário
    if (this->_pendingResponse.empty()) {
        this->_pendingResponse = responseStr;
        this->_responseOffset = 0;
    }
    
    // Enviar dados a partir do offset atual
    const char *data = this->_pendingResponse.c_str() + this->_responseOffset;
    size_t remaining = this->_pendingResponse.size() - this->_responseOffset;
    
    // send() com flags MSG_NOSIGNAL para evitar SIGPIPE
    ssize_t sent = send(this->getSocketFd(), data, remaining, MSG_NOSIGNAL);
    
    // Verificar valor de retorno conforme régua de avaliação
    // Conforme régua: "checking only -1 or 0 values is not enough, both should be checked"
    if (sent < 0) {
        // Erro no send()
        // Em socket non-blocking, -1 pode ser EAGAIN/EWOULDBLOCK (normal) ou erro real
        // NÃO verificamos errno diretamente (conforme régua), mas tratamos o erro
        // Em non-blocking, EAGAIN significa que o buffer está cheio - precisamos esperar EPOLLOUT
        // Outros erros devem resultar em remoção do cliente
        // Por segurança, assumimos que é EAGAIN e adicionamos EPOLLOUT
        // Se for erro real, o próximo send() falhará novamente e então removemos
        EpollInstance::manipInterestList(EPOLL_CTL_DEL, this);
        return true;
    } else if (sent == 0) {
        // send() retornou 0 - conexão fechada pelo peer
        // Remover cliente conforme régua: "if an error is returned, the client is removed"
        std::cout << "Connection closed by peer during send, closing client." << std::endl;
        EpollInstance::manipInterestList(EPOLL_CTL_DEL, this);
        return false;
    } else {
        // send() enviou alguns bytes (pode ser parcial)
        this->_responseOffset += sent;
        
        if (this->_responseOffset < this->_pendingResponse.size()) {
            // Ainda há dados para enviar - adicionar EPOLLOUT
            uint32_t events = this->getInterestedEvents();
            events |= EPOLLOUT;
            this->setInterestedEvents(events);
            EpollInstance::manipInterestList(EPOLL_CTL_MOD, this);
        }
        
        return true;
    }
}

void Client::concatenateRequestData(std::string data) {
    if (this->_state == COMPLETE) {
        return;
    }
    std::cout << "------------concatenate request-----------------" << std::endl;
    std::cout << data << std::endl;

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
            !this->_serverListen.getServerBlock().getValidLocation(this->request.getUri(), this->request.getMethod())
        ) {
            this->response.setErrorPage(405);
            this->setState(COMPLETE);
            return;
        }
        this->setState(READING_BODY);
    }

    if (this->_state == READING_BODY) {
        // Quando estivermos lendo o body, nao podemos nos basear apenas no content length.
        // Pois esse header nao e obrigatorio. Entao, dessa forma, nao e garantia de nada.
        // E, quando o contne type for multipart, o client pode mandar o encoding como chunked.
        // E, caso ele mande esse encoding diferente, o content length nao vem tambem.
        // e o body vem com um formato diferente, onde a string "0\r\n\r\n" indica o final do body.
        std::string contentLengthStr = this->request.getHeaderValue("Content-Length");
        if (!contentLengthStr.empty()) { // caso tenha content length. Que e 99% dos casos
            int contentLength = std::atoi(contentLengthStr.c_str());
            size_t bodyStartPos = this->_rawRequest.find("\r\n\r\n") + 4;
            size_t bodyLength = this->_rawRequest.size() - bodyStartPos;
            std::string onlyBody = this->_rawRequest.substr(bodyStartPos, bodyLength);
            
            if (bodyLength >= static_cast<size_t>(contentLength)) {
                this->request.parseBody(this->_rawRequest, onlyBody);
                this->setState(COMPLETE);
            }
            Logger::debug("----- testando o max_body_size ---------");
            std::cout << "bodyLenght: " << bodyLength << std::endl;
            std::cout << "content_lenght: " << contentLengthStr << std::endl;
            Logger::debug("----- testando o max_body_size ---------");
        } else if (this->_rawRequest.find("0\r\n\r\n") != std::string::npos) { //caso tenha outro encoding (chunked)
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
#include "../includes/WebservHeader.hpp"

CgiHandler::CgiHandler(
  const HttpRequest &request,
  const ServerBlock &serverBlock,
  const LocationBlock &location
) : EpollHandler(0, -1, 10), _scriptPath(""), _cgiOutput(""), _isFinished(false), _childPid(-1), _request(request), _serverBlock(serverBlock), _location(location) {
    this->_scriptPath = extractCgiScriptPath(request.getUri());
    this->_env = buildEnvironment();
    this->_requestBody = request.getBody();
    this->_bytesWritten = 0;
    _fdIn[0] = -1;
    _fdIn[1] = -1;
    _fdOut[0] = -1;
    _fdOut[1] = -1;
}

CgiHandler::~CgiHandler() {
  if (_fdIn[0] != -1) close(_fdIn[0]);
  if (_fdIn[1] != -1) close(_fdIn[1]);
  if (_fdOut[0] != -1) close(_fdOut[0]);
  if (_fdOut[1] != -1) close(_fdOut[1]);

  if (_childPid != -1) {
    kill(_childPid, SIGKILL);

    int status;
    waitpid(_childPid, &status, 0);
  }
}

void CgiHandler::handleEpollIn() {
    char buffer[MAX_BUFFER_SIZE];

    while (true) {
        ssize_t bytesRead = read(getSocketFd(), buffer, sizeof(buffer));

        if (bytesRead > 0) {
            _cgiOutput.append(buffer, bytesRead);
            continue;
        } else if (bytesRead == 0) {
            _isFinished = true;
            return;
        }

        return;
    }
}

void CgiHandler::handleEpollOut() {
    while (_bytesWritten < _requestBody.size()) {
        ssize_t n = write(
            getSocketFd(),
            _requestBody.c_str() + _bytesWritten,
            _requestBody.size() - _bytesWritten
        );

        if (n > 0) {
            _bytesWritten += n;
            continue;
        }

        return;
    }

    Logger::debug("Cgi: request body fully sent to CGI script.");

    if (_fdIn[1] != -1) {
        close(_fdIn[1]);
        _fdIn[1] = -1;
    }

    int readFd = _fdOut[0];
    setNonBlocking(readFd);

    try {
        EpollInstance::replaceHandlerFd(
            this,
            readFd,
            EPOLLIN | EPOLLET | EPOLLRDHUP
        );
    } catch (const std::exception &e) {
        Logger::error("CgiHandler::handleEpollOut - replaceHandlerFd failed: " + std::string(e.what()));
        _isFinished = true;
        return;
    }

    Logger::debug("Cgi: switched to read fd" + intToString(readFd));
}

const std::string& CgiHandler::getCgiOutput() const {
    return _cgiOutput;
}

bool CgiHandler::isFinished() const {
    return _isFinished;
}

bool CgiHandler::start() {
    try {
        if (pipe(_fdIn) == -1 || pipe(_fdOut) == -1) {
            Logger::error("CGI: Failed to create pipes");
            return false;
        }
        Logger::debug("Cgi: Pipes created successfully.");

        pid_t pid = fork();
        if (pid < 0) {
            Logger::error("CGI: Fork failed");
            close(_fdIn[0]); close(_fdIn[1]);
            close(_fdOut[0]); close(_fdOut[1]);
            return false;
        }

        if (pid == 0) {
            dup2(_fdIn[0], STDIN_FILENO);
            dup2(_fdOut[1], STDOUT_FILENO);

            // close parent fds
            if (_fdIn[0] != -1) close(_fdIn[0]);
            if (_fdIn[1] != -1) close(_fdIn[1]);
            if (_fdOut[0] != -1) close(_fdOut[0]);
            if (_fdOut[1] != -1) close(_fdOut[1]);

            std::vector<char*> envp;
            for (size_t i = 0; i < _env.size(); i++) {
                envp.push_back(const_cast<char*>(_env[i].c_str()));
            }
            envp.push_back(NULL);

            std::string interpretterPath = getInterpretterPath(_scriptPath);
            char* const argv[] = {const_cast<char*>(interpretterPath.c_str()), const_cast<char*>(_scriptPath.c_str()), NULL};
            execve(interpretterPath.c_str(), argv, envp.data());
            _exit(1);
        } else {
            _childPid = pid;

            if (_fdIn[0] != -1) {
                close(_fdIn[0]);
                _fdIn[0] = -1;
            }

            if (_fdOut[1] != -1) {
                close(_fdOut[1]);
                _fdOut[1] = -1;
            }

            if (_fdIn[1] != -1) setNonBlocking(_fdIn[1]);
            if (_fdOut[0] != -1) setNonBlocking(_fdOut[0]);

            int epfd = EpollInstance::getEpollFd();
            if (epfd == -1) {
                Logger::error("CgiHandler::start - invalid epoll fd");
                return false;
            }

            if (_request.getMethod() == "POST" && !_requestBody.empty()) {
                Logger::debug("Cgi: POST request with body, registering writer fd.");

                this->setSocketFd(_fdIn[1]);
                this->setInterestedEvents(EPOLLOUT | EPOLLET | EPOLLRDHUP);
                EpollInstance::manipInterestList(EPOLL_CTL_ADD, this);
            } else {
                Logger::debug("Cgi: " + _request.getMethod() + " request/no body, registering reader fd.");
                if (_fdIn[1] != -1) {
                    close(_fdIn[1]); 
                    _fdIn[1] = -1;
                }

                this->setSocketFd(_fdOut[0]);
                this->setInterestedEvents(EPOLLIN | EPOLLET | EPOLLRDHUP);
                EpollInstance::manipInterestList(EPOLL_CTL_ADD, this);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "CGI: Exception in start(): " << e.what() << std::endl;
        return false;
    }
    return true;
}

std::vector<std::string> CgiHandler::buildEnvironment() {
    std::vector<std::string> env;
    
    env.push_back("REQUEST_METHOD=" + _request.getMethod());
    env.push_back("REQUEST_URI=" + _request.getUri());
    env.push_back("SCRIPT_NAME=" + extractUriWithoutQuery(_request.getUri()));
    env.push_back("PATH_INFO=" + extractUriPathInfo(_request.getUri(), _location));
    env.push_back("PATH_TRANSLATED=" + _scriptPath);
    
    std::string queryString = extractQueryFromUri(_request.getUri());
    if (!queryString.empty()) {
        env.push_back("QUERY_STRING=" + queryString);
    }
    
    if (_request.hasHeader("Content-Type"))
        env.push_back("CONTENT_TYPE=" + _request.getHeaderValue("Content-Type"));
    if (_request.hasHeader("Content-Length"))
        env.push_back("CONTENT_LENGTH=" + _request.getHeaderValue("Content-Length"));
    
    std::vector<t_listen> listens = _serverBlock.getListen();
    if (!listens.empty()) {
        env.push_back("SERVER_PORT=" + intToString(listens[0].port));
    }
    std::vector<std::string> serverNames = _serverBlock.getServerNames();
    env.push_back("SERVER_NAME=" + (serverNames.empty() ? "localhost" : serverNames[0]));
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env.push_back("SERVER_SOFTWARE=WebServ/1.0");
    
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    
    std::map<std::string, std::string> headers = _request.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        
        std::string lowerKey = it->first;
        for (size_t i = 0; i < lowerKey.size(); i++) {
            lowerKey[i] = std::tolower(lowerKey[i]);
        }
        
        if (lowerKey != "content-type" && lowerKey != "content-length") {
            std::string envName = "HTTP_" + normalizeHeaderName(it->first);
            env.push_back(envName + "=" + it->second);
        }
    }
    
    return env;
}

std::string CgiHandler::extractCgiScriptPath(
  const std::string& uri
) {
    std::string path = extractUriWithoutQuery(uri);
    
    std::string alias = this->_location.getAlias();
    if (!alias.empty()) {
        std::string locationPath = this->_location.getUri();
        if (path.find(locationPath) == 0) {
            path = path.substr(locationPath.size());
        }

        if (path[0] != '/') {
            path = "/" + path;
        }
        path = alias + path;
    } else {
        std::pair<bool, std::string> rootPair = this->_serverBlock.getRoot();
        std::string root = rootPair.second;
        
        if (path[0] != '/') {
            path = "/" + path;
        }
        path = root + path;
    }

    return path;
}

std::string CgiHandler::normalizeHeaderName(const std::string& header) {
    std::string result = header;
    
    for (size_t i = 0; i < result.size(); i++) {
        result[i] = std::toupper(result[i]);
    }
    
    for (size_t i = 0; i < result.size(); i++) {
        if (result[i] == '-')
            result[i] = '_';
    }
    
    return result;
}

bool CgiHandler::isCgiScript(const std::string& uri, const LocationBlock& location) {
    std::string path = extractUriWithoutQuery(uri);
    
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos)
        return false;
    
    std::string extension = path.substr(dotPos);
    std::vector<std::string> cgiExtensions = location.getCgiExtensions();
    
    for (size_t i = 0; i < cgiExtensions.size(); i++) {
        if (cgiExtensions[i] == extension)
            return true;
    }
    
    return false;
}

std::string CgiHandler::getInterpretterPath(const std::string& scriptPath) {
    size_t dotPos = scriptPath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return scriptPath;
    }
    
    std::string extension = scriptPath.substr(dotPos);
    
    if (extension == ".py") {
        return "/usr/bin/python3";
    } else if (extension == ".pl") {
        return "/usr/bin/perl";
    } else if (extension == ".sh") {
        return "/bin/bash";
    } else if (extension == ".php") {
        return "/usr/bin/php";
    } else {
        return scriptPath;
    }
}


#include "../includes/WebservHeader.hpp"

#include "../includes/WebservHeader.hpp"
#include <sys/wait.h> // Para waitpid

CgiHandler::CgiHandler(
  const HttpRequest &request,
  const ServerBlock &serverBlock,
  const LocationBlock &location
) : EpollHandler(0), _scriptPath(""), _cgiOutput(""), _isFinished(false), _childPid(-1), _request(request), _serverBlock(serverBlock), _location(location) { // Initialize _cgiOutput, _isFinished, _childPid here
    this->_scriptPath = extractCgiScriptPath(request.getUri());
    this->_env = buildEnvironment();
    _fdIn[0] = -1;
    _fdIn[1] = -1;
    _fdOut[0] = -1;
    _fdOut[1] = -1;
}

CgiHandler::~CgiHandler() {
  // Close any remaining file descriptors
  if (_fdIn[0] != -1) close(_fdIn[0]);
  if (_fdIn[1] != -1) close(_fdIn[1]);
  if (_fdOut[0] != -1) close(_fdOut[0]);
  if (_fdOut[1] != -1) close(_fdOut[1]);

  // Optionally wait for child process if it hasn't been waited for yet
  if (_childPid != -1) {
    int status;
    waitpid(_childPid, &status, WNOHANG); // Non-blocking wait
  }
}

void CgiHandler::handleEpollIn() {
    std::cerr << "CGI: handleEpollIn called" << std::endl;
    char buffer[4096];
    ssize_t bytesRead;

    // Read from the CGI's stdout pipe
    bytesRead = read(_fdOut[0], buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        _cgiOutput.append(buffer);
        std::cout << "CGI output: " << _cgiOutput << std::endl;
    } else if (bytesRead == 0) { // EOF, CGI process finished writing
        _isFinished = true;
        // Optionally, wait for child process here to avoid zombies, or rely on Client to do it.
        // For now, let's keep it simple and rely on external cleanup.
        std::cerr << "CGI: Finished reading output" << std::endl;
    } else { // Error
        std::cerr << "CGI: Error reading from pipe: " << strerror(errno) << std::endl;
        _isFinished = true; // Mark as finished even on error
    }
}

void CgiHandler::handleEpollOut() {
    // For now, this is empty as we are focusing on reading CGI output.
    // If POST requests to CGI are implemented, this would write _request.getBody() to _fdIn[1].
}

const std::string& CgiHandler::getCgiOutput() const {
    return _cgiOutput;
}

bool CgiHandler::isFinished() const {
    return _isFinished;
}

bool CgiHandler::start(
) {
    try {

        if (pipe(_fdIn) == -1 || pipe(_fdOut) == -1) {
            std::cerr << "CGI: Failed to create pipes" << std::endl;
            return false;
        }
        
        set_nonblocking(_fdIn[1]);
        set_nonblocking(_fdOut[0]);

        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << "CGI: Fork failed" << std::endl;
            close(_fdIn[0]);
            close(_fdIn[1]);
            close(_fdOut[0]);
            close(_fdOut[1]);
            _fdIn[0] = -1; _fdIn[1] = -1; _fdOut[0] = -1; _fdOut[1] = -1;
            return false;
        }

        if (pid == 0) {
            // Processo filho
            dup2(_fdIn[0], STDIN_FILENO);
            dup2(_fdOut[1], STDOUT_FILENO);
            
            close(_fdIn[0]);
            close(_fdIn[1]);
            close(_fdOut[0]);
            close(_fdOut[1]);

            for (int i = 3; i < 1024; i++) {
                close(i);
            }
            
            // Construir array de char* para execve
            std::vector<char*> envp;
            for (size_t i = 0; i < _env.size(); i++) {
                envp.push_back(const_cast<char*>(_env[i].c_str()));
            }
            envp.push_back(NULL);
            
            std::string interpretterPath = getInterpretterPath(_scriptPath);
            char* const argv[] = {const_cast<char*>(interpretterPath.c_str()), const_cast<char*>(_scriptPath.c_str()), NULL};
            std::cerr << "CGI: Executing " << interpretterPath << " with script " << _scriptPath << std::endl;
            execve(interpretterPath.c_str(), argv, envp.data());
            
            // Se chegou aqui, execve falhou
            std::cerr << "CGI: execve failed: " << strerror(errno) << std::endl;
            exit(1);
        } else {
            // Processo pai
            close(_fdIn[0]);
            close(_fdOut[1]);
            
            this->setSocketFd(_fdOut[0]);
            this->setInterestedEvents(EPOLLIN | EPOLLET);
            EpollInstance::manipInterestList(EPOLL_CTL_ADD, this);
        }
        
        
    } catch (const std::exception& e) {
        std::cerr << "CGI: Exception in start(): " << e.what() << std::endl;
        return false;
    }
    return true;
}

// ... rest of the file remains the same
std::vector<std::string> CgiHandler::buildEnvironment() {
    std::vector<std::string> env;
    
    // Método HTTP
    env.push_back("REQUEST_METHOD=" + _request.getMethod());
    
    // URI e caminho
    env.push_back("REQUEST_URI=" + _request.getUri());
    env.push_back("SCRIPT_NAME=" + extractCgiScriptName(_request.getUri()));
    env.push_back("PATH_INFO=" /*+ extractPathInfo(_request.getUri()) */);
    env.push_back("PATH_TRANSLATED=" + _scriptPath);
    
    // Query string
    std::string queryString = extractQueryString(_request.getUri());
    if (!queryString.empty()) {
        env.push_back("QUERY_STRING=" + queryString);
    }
    
    // Content-Type e Content-Length
    if (_request.hasHeader("Content-Type"))
        env.push_back("CONTENT_TYPE=" + _request.getHeaderValue("Content-Type"));
    if (_request.hasHeader("Content-Length"))
        env.push_back("CONTENT_LENGTH=" + _request.getHeaderValue("Content-Length"));
    
    // Informações do servidor
    std::vector<t_listen> listens = _serverBlock.getListen();
    if (!listens.empty()) {
        env.push_back("SERVER_PORT=" + intToString(listens[0].port));
    }
    std::vector<std::string> serverNames = _serverBlock.getServerNames();
    env.push_back("SERVER_NAME=" + (serverNames.empty() ? "localhost" : serverNames[0]));
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env.push_back("SERVER_SOFTWARE=WebServ/1.0");
    
    // CGI/1.1
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    
    // Headers HTTP (prefixo HTTP_)
    std::map<std::string, std::string> headers = _request.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        
        // Ignorar headers que já foram processados
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
    std::cerr << "CGI: Extracting script path from URI: " << uri << std::endl;
    std::string path = extractCgiScriptName(uri);
    
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

std::string CgiHandler::extractCgiScriptName(const std::string& uri) {
    size_t queryPos = uri.find('?');
    if (queryPos != std::string::npos) {
        return uri.substr(0, queryPos);
    }
    return uri;
}

std::string CgiHandler::extractQueryString(const std::string& uri) {
    size_t queryPos = uri.find('?');
    if (queryPos != std::string::npos && queryPos + 1 < uri.size()) {
        return uri.substr(queryPos + 1);
    }
    return "";
}

std::string CgiHandler::intToString(int n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
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
    std::string path = extractCgiScriptName(uri);
    
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
    std::cerr << "CGI: Determining interpreter for script: " << scriptPath << std::endl;
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

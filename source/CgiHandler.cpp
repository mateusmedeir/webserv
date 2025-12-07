#include "../includes/CgiHandler.hpp"
#include "../includes/CgiProcess.hpp"
#include "../includes/CgiPipeHandler.hpp"
#include "../includes/RunTime.hpp"
#include "../includes/EpollInstance.hpp"
#include "../includes/Client.hpp"
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

// Inicialização de variáveis estáticas
std::map<pid_t, CgiProcess*> CgiHandler::_activeProcesses;
std::map<int, CgiProcess*> CgiHandler::_clientToProcess;
std::map<int, CgiPipeHandler*> CgiHandler::_pipeHandlers;

bool CgiHandler::shouldExecuteCgi(const std::string& uri, const LocationBlock& location) {
    // Remover query string se houver
    std::string path = uri;
    size_t queryPos = path.find('?');
    if (queryPos != std::string::npos) {
        path = path.substr(0, queryPos);
    }
    
    // Extrair extensão
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos)
        return false;
    
    std::string extension = path.substr(dotPos);
    std::vector<std::string> cgiExtensions = location.getCgiExtensions();
    
    // Verificar se extensão está na lista
    for (size_t i = 0; i < cgiExtensions.size(); i++) {
        if (cgiExtensions[i] == extension)
            return true;
    }
    
    return false;
}

std::string CgiHandler::getCgiScriptPath(const std::string& uri,
                                           const ServerBlock& serverBlock,
                                           const LocationBlock& location) {
    std::string path = uri;
    
    // Remover query string se houver
    size_t queryPos = path.find('?');
    if (queryPos != std::string::npos) {
        path = path.substr(0, queryPos);
    }
    
    // Usar alias se disponível
    std::string alias = location.getAlias();
    if (!alias.empty()) {
        // Remover o prefixo do location da URI
        std::string locationPath = location.getUri();
        if (path.find(locationPath) == 0) {
            path = path.substr(locationPath.size());
        }
        // Construir caminho completo
        if (path[0] != '/') {
            path = "/" + path;
        }
        path = alias + path;
    } else {
        // Usar root do server block
        std::pair<bool, std::string> rootPair = serverBlock.getRoot();
        std::string root = rootPair.second;
        
        if (path[0] != '/') {
            path = "/" + path;
        }
        path = root + path;
    }
    
    return path;
}

std::string CgiHandler::extractScriptName(const std::string& uri) {
    size_t queryPos = uri.find('?');
    if (queryPos != std::string::npos) {
        return uri.substr(0, queryPos);
    }
    return uri;
}

std::string CgiHandler::extractPathInfo(const std::string& uri) {
    (void)uri; // Parâmetro não usado ainda
    // Path info é a parte após o script name
    // Por enquanto, retornamos vazio
    // TODO: Implementar parsing mais sofisticado se necessário
    return "";
}

std::string CgiHandler::extractQueryString(const std::string& uri) {
    size_t queryPos = uri.find('?');
    if (queryPos != std::string::npos) {
        return uri.substr(queryPos + 1);
    }
    return "";
}

std::string CgiHandler::normalizeHeaderName(const std::string& header) {
    std::string result = header;
    
    // Converter para maiúsculas
    for (size_t i = 0; i < result.size(); i++) {
        result[i] = std::toupper(result[i]);
    }
    
    // Substituir hífens por underscores
    for (size_t i = 0; i < result.size(); i++) {
        if (result[i] == '-')
            result[i] = '_';
    }
    
    return result;
}

std::string CgiHandler::intToString(int n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

std::vector<std::string> CgiHandler::buildEnvironment(const HttpRequest& req,
                                                         const ServerBlock& serverBlock,
                                                         const LocationBlock& location,
                                                         const std::string& scriptPath) {
    (void)location; // Parâmetro não usado ainda
    std::vector<std::string> env;
    
    // Método HTTP
    env.push_back("REQUEST_METHOD=" + req.getMethod());
    
    // URI e caminho
    env.push_back("REQUEST_URI=" + req.getUri());
    env.push_back("SCRIPT_NAME=" + extractScriptName(req.getUri()));
    env.push_back("PATH_INFO=" + extractPathInfo(req.getUri()));
    env.push_back("PATH_TRANSLATED=" + scriptPath);
    
    // Query string
    std::string queryString = extractQueryString(req.getUri());
    if (!queryString.empty()) {
        env.push_back("QUERY_STRING=" + queryString);
    }
    
    // Content-Type e Content-Length
    if (req.hasHeader("Content-Type")) {
        env.push_back("CONTENT_TYPE=" + req.getHeaderValue("Content-Type"));
    }
    if (req.hasHeader("Content-Length")) {
        env.push_back("CONTENT_LENGTH=" + req.getHeaderValue("Content-Length"));
    }
    
    // Informações do servidor
    std::vector<t_listen> listens = serverBlock.getListen();
    if (!listens.empty()) {
        env.push_back("SERVER_PORT=" + intToString(listens[0].port));
    }
    std::vector<std::string> serverNames = serverBlock.getServerNames();
    env.push_back("SERVER_NAME=" + (serverNames.empty() ? "localhost" : serverNames[0]));
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env.push_back("SERVER_SOFTWARE=WebServ/1.0");
    
    // CGI/1.1
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    
    // Headers HTTP (prefixo HTTP_)
    std::map<std::string, std::string> headers = req.getHeaders();
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

std::string CgiHandler::findBestLocationMatch(const std::string& uri,
                                               const ServerBlock& serverBlock,
                                               LocationBlock& location) {
    std::map<std::string, LocationBlock> locations = serverBlock.getLocations();
    std::string bestMatch = "";
    
    for (std::map<std::string, LocationBlock>::const_iterator it = locations.begin();
         it != locations.end(); ++it) {
        const std::string &path = it->first;
        if (uri.compare(0, path.size(), path) == 0) {
            if (path.size() > bestMatch.size()) {
                bestMatch = path;
                location = it->second;
            }
        }
    }
    
    return bestMatch;
}

bool CgiHandler::executeCgi(const HttpRequest& req,
                            const ServerBlock& serverBlock,
                            const LocationBlock& location,
                            std::string& output) {
    // Verificar se precisa executar CGI
    if (!shouldExecuteCgi(req.getUri(), location)) {
        return false;
    }
    
    // Obter caminho do script
    std::string scriptPath = getCgiScriptPath(req.getUri(), serverBlock, location);
    
    // Verificar se arquivo existe e é executável
    struct stat fileStat;
    if (stat(scriptPath.c_str(), &fileStat) != 0) {
        std::cerr << "CGI: Script not found: " << scriptPath << std::endl;
        return false;
    }
    
    if (!S_ISREG(fileStat.st_mode)) {
        std::cerr << "CGI: Not a regular file: " << scriptPath << std::endl;
        return false;
    }
    
    // Construir ambiente
    std::vector<std::string> env = buildEnvironment(req, serverBlock, location, scriptPath);
    
    // Criar processo CGI (sem clientFd para execução síncrona)
    CgiProcess* process = new CgiProcess(scriptPath, env, -1);
    
    if (!process->start()) {
        delete process;
        return false;
    }
    
    // Escrever body se houver
    if (!req.getBody().empty()) {
        process->writeToStdin(req.getBody());
    }
    
    // Ler resposta (bloqueante para execução síncrona)
    // Em execução síncrona, esperamos até o processo terminar
    while (!process->isFinished() && !process->isTimedOut()) {
        process->readFromStdout();
        usleep(10000); // 10ms
    }
    
    if (process->isTimedOut()) {
        process->killProcess();
        delete process;
        return false;
    }
    
    // Obter output
    output = process->getOutput();
    
    // Limpar
    delete process;
    
    return true;
}

bool CgiHandler::executeCgiAsync(const HttpRequest& req,
                                  const ServerBlock& serverBlock,
                                  const LocationBlock& location,
                                  int clientFd) {
    // Verificar se precisa executar CGI
    if (!shouldExecuteCgi(req.getUri(), location)) {
        return false;
    }
    
    // Obter caminho do script
    std::string scriptPath = getCgiScriptPath(req.getUri(), serverBlock, location);
    std::cout << "CGI: Executing script: " << scriptPath << std::endl;
    
    // Verificar se arquivo existe e é executável
    struct stat fileStat;
    if (stat(scriptPath.c_str(), &fileStat) != 0) {
        std::cerr << "CGI: Script not found: " << scriptPath << std::endl;
        return false;
    }
    
    if (!S_ISREG(fileStat.st_mode)) {
        std::cerr << "CGI: Not a regular file: " << scriptPath << std::endl;
        return false;
    }
    
    // Construir ambiente
    std::vector<std::string> env = buildEnvironment(req, serverBlock, location, scriptPath);
    
    // Criar processo CGI
    CgiProcess* process = new CgiProcess(scriptPath, env, clientFd);
    
    if (!process->start()) {
        std::cerr << "CGI: Failed to start process" << std::endl;
        delete process;
        return false;
    }
    
    std::cout << "CGI: Process started with PID: " << process->getPid() << std::endl;
    
    // Armazenar body para escrita assíncrona
    if (!req.getBody().empty()) {
        process->setRemainingBody(req.getBody());
    } else {
        // Para GET sem body, fechar pipe de entrada imediatamente
        // para que o processo CGI não fique travado esperando EOF
        process->closeInputPipe();
    }
    
    // Registrar processo
    _activeProcesses[process->getPid()] = process;
    _clientToProcess[clientFd] = process;
    
    // Handler para pipe de entrada (escrita) - apenas se houver body E pipe ainda aberto
    if (process->getPipeInFd() != -1 && !req.getBody().empty()) {
        CgiPipeHandler* inputHandler = new CgiPipeHandler(process->getPipeInFd(), clientFd, true);
        _pipeHandlers[process->getPipeInFd()] = inputHandler;
        EpollInstance::manipInterestList(EPOLL_CTL_ADD, inputHandler);
        
        // Adicionar evento EPOLLOUT imediatamente pois há body para enviar
        inputHandler->setInterestedEvents(EPOLLOUT);
        EpollInstance::manipInterestList(EPOLL_CTL_MOD, inputHandler);
    }
    
    // Handler para pipe de saída (leitura) - sempre necessário
    if (process->getPipeOutFd() != -1) {
        CgiPipeHandler* outputHandler = new CgiPipeHandler(process->getPipeOutFd(), clientFd, false);
        _pipeHandlers[process->getPipeOutFd()] = outputHandler;
        EpollInstance::manipInterestList(EPOLL_CTL_ADD, outputHandler);
        std::cout << "CGI: Output handler registered with EPOLLIN for fd " << process->getPipeOutFd() << std::endl;
    }
    
    std::cout << "CGI: Handlers registered. PipeIn: " << process->getPipeInFd() 
              << ", PipeOut: " << process->getPipeOutFd() << std::endl;
    
    return true;
}

void CgiHandler::handleCgiPipeIn(int fd, Client* client) {
    CgiProcess* process = getProcessByClient(client->getSocketFd());
    if (!process || process->getPipeInFd() != fd) {
        std::cerr << "CGI: handleCgiPipeIn - process not found or wrong fd" << std::endl;
        return;
    }
    
    std::cout << "CGI: handleCgiPipeIn called for fd " << fd << std::endl;
    std::string body = process->getRemainingBody();
    std::cout << "CGI: Remaining body size: " << body.size() << std::endl;
    
    if (!body.empty()) {
        // Tentar escrever no pipe (writeToStdin atualiza remainingBody automaticamente)
        bool wroteAll = process->writeToStdin(body);
        std::cout << "CGI: writeToStdin result: " << (wroteAll ? "all written" : "partial/none") << std::endl;
        
        if (wroteAll) {
            // Tudo foi escrito - limpar remaining body
            process->setRemainingBody("");
            
            // Remover EPOLLOUT do handler (pipe foi fechado)
            // IMPORTANTE: Verificar se o pipe ainda está aberto antes de modificar epoll
            if (process->getPipeInFd() != -1) {
                std::map<int, CgiPipeHandler*>::iterator it = _pipeHandlers.find(fd);
                if (it != _pipeHandlers.end()) {
                    CgiPipeHandler* handler = it->second;
                    handler->setInterestedEvents(0);
                    EpollInstance::manipInterestList(EPOLL_CTL_MOD, handler);
                }
            }
            
            // Após fechar pipe de entrada, tentar ler do pipe de saída imediatamente
            // O processo CGI pode ter começado a escrever
            if (process->getPipeOutFd() != -1) {
                // Tentar ler imediatamente
                process->readFromStdout();
                
                // Verificar se processo terminou
                if (process->isFinished()) {
                    std::string cgiOutput = process->getOutput();
                    if (!cgiOutput.empty()) {
                        std::cout << "CGI: Got response immediately after closing input pipe" << std::endl;
                        client->getResponse().processCgiResponse(cgiOutput);
                        std::string responseStr = client->getResponse().toString();
                        cleanupClientProcess(client->getSocketFd());
                        client->sendResponse(responseStr);
                        return;
                    }
                }
                
                // Se ainda não terminou, verificar se processo realmente terminou (pode ter terminado mas pipe ainda não fechou)
                int status;
                pid_t waitResult = waitpid(process->getPid(), &status, WNOHANG);
                if (waitResult == process->getPid()) {
                    // Processo terminou - ler qualquer dado restante
                    process->readFromStdout();
                    std::string cgiOutput = process->getOutput();
                    if (!cgiOutput.empty()) {
                        std::cout << "CGI: Process terminated, got response" << std::endl;
                        client->getResponse().processCgiResponse(cgiOutput);
                        std::string responseStr = client->getResponse().toString();
                        cleanupClientProcess(client->getSocketFd());
                        client->sendResponse(responseStr);
                        return;
                    }
                }
            }
        }
        // Se escreveu parcialmente, o remainingBody já foi atualizado pelo writeToStdin
        // e o epoll vai chamar novamente quando o pipe estiver pronto
    } else {
        std::cout << "CGI: No body to write, closing pipe" << std::endl;
        // Sem mais dados - remover EPOLLOUT
        std::map<int, CgiPipeHandler*>::iterator it = _pipeHandlers.find(fd);
        if (it != _pipeHandlers.end()) {
            CgiPipeHandler* handler = it->second;
            handler->setInterestedEvents(0);
            EpollInstance::manipInterestList(EPOLL_CTL_MOD, handler);
        }
    }
}

void CgiHandler::handleCgiPipeOut(int fd, Client* client) {
    CgiProcess* process = getProcessByClient(client->getSocketFd());
    if (!process || process->getPipeOutFd() != fd) {
        std::cerr << "CGI: handleCgiPipeOut - process not found or wrong fd" << std::endl;
        return;
    }
    
    std::cout << "CGI: handleCgiPipeOut called for fd " << fd << std::endl;
    process->readFromStdout();
    
    // Verificar se processo terminou OU se pipe foi fechado (EOF)
    // Se pipe foi fechado mas ainda há dados, processar resposta
    if (process->isFinished()) {
        // Processo terminou - processar resposta
        std::string cgiOutput = process->getOutput();
        std::cout << "CGI: Process finished, output length: " << cgiOutput.size() << std::endl;
        
        if (cgiOutput.empty()) {
            std::cerr << "CGI: Empty output from process" << std::endl;
            client->getResponse().setStatus(502, "Bad Gateway");
            client->getResponse().setBody("<h1>502 Bad Gateway</h1>", "text/html");
        } else {
            client->getResponse().processCgiResponse(cgiOutput);
        }
        
        // Preparar resposta para envio
        std::string responseStr = client->getResponse().toString();
        
        // Limpar processo e handlers de pipe ANTES de enviar resposta
        cleanupClientProcess(client->getSocketFd());
        
        // Enviar resposta
        std::cout << "CGI: Sending response to client" << std::endl;
        client->sendResponse(responseStr);
    } else {
        // Processo ainda rodando - mas pode ter terminado após escrever
        // Verificar se processo realmente terminou (sem bloquear)
        int status;
        pid_t waitResult = waitpid(process->getPid(), &status, WNOHANG);
        if (waitResult == process->getPid()) {
            // Processo terminou - processar resposta mesmo que não tenhamos recebido EOF ainda
            std::cout << "CGI: Process " << process->getPid() << " terminated, processing response" << std::endl;
            
            // Ler qualquer dado restante
            process->readFromStdout();
            
            std::string cgiOutput = process->getOutput();
            if (cgiOutput.empty()) {
                std::cerr << "CGI: Empty output from process" << std::endl;
                client->getResponse().setStatus(502, "Bad Gateway");
                client->getResponse().setBody("<h1>502 Bad Gateway</h1>", "text/html");
            } else {
                client->getResponse().processCgiResponse(cgiOutput);
            }
            
            std::string responseStr = client->getResponse().toString();
            cleanupClientProcess(client->getSocketFd());
            std::cout << "CGI: Sending response to client" << std::endl;
            client->sendResponse(responseStr);
            std::cout << "CGI: Response sent, client will be closed when send completes" << std::endl;
        } else {
            std::cout << "CGI: Process still running, waiting for more data" << std::endl;
        }
    }
}

void CgiHandler::cleanupProcess(pid_t pid) {
    std::map<pid_t, CgiProcess*>::iterator it = _activeProcesses.find(pid);
    if (it != _activeProcesses.end()) {
        CgiProcess* process = it->second;
        
        // Remover handlers de pipe
        if (process->getPipeInFd() != -1) {
            cleanupPipeHandler(process->getPipeInFd());
        }
        if (process->getPipeOutFd() != -1) {
            cleanupPipeHandler(process->getPipeOutFd());
        }
        
        // Remover do mapeamento cliente
        _clientToProcess.erase(process->getClientFd());
        
        // Deletar processo
        delete process;
        _activeProcesses.erase(it);
    }
}

void CgiHandler::cleanupClientProcess(int clientFd) {
    std::map<int, CgiProcess*>::iterator it = _clientToProcess.find(clientFd);
    if (it != _clientToProcess.end()) {
        CgiProcess* process = it->second;
        
        // Remover handlers de pipe
        if (process->getPipeInFd() != -1) {
            cleanupPipeHandler(process->getPipeInFd());
        }
        if (process->getPipeOutFd() != -1) {
            cleanupPipeHandler(process->getPipeOutFd());
        }
        
        // Remover do mapeamento de processos
        _activeProcesses.erase(process->getPid());
        
        // Deletar processo
        delete process;
        _clientToProcess.erase(it);
    }
}

void CgiHandler::cleanupPipeHandler(int pipeFd) {
    std::map<int, CgiPipeHandler*>::iterator it = _pipeHandlers.find(pipeFd);
    if (it != _pipeHandlers.end()) {
        CgiPipeHandler* handler = it->second;
        
        // Remover do epoll
        EpollInstance::manipInterestList(EPOLL_CTL_DEL, handler);
        
        // Deletar handler
        delete handler;
        _pipeHandlers.erase(it);
    }
}

void CgiHandler::cleanupZombieProcesses() {
    pid_t pid;
    int status;
    
    // waitpid com WNOHANG para não bloquear
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        std::map<pid_t, CgiProcess*>::iterator it = _activeProcesses.find(pid);
        
        if (it != _activeProcesses.end()) {
            CgiProcess* process = it->second;
            
            // Processo terminou normalmente
            if (WIFEXITED(status)) {
                int exitCode = WEXITSTATUS(status);
                if (exitCode != 0) {
                    std::cout << "CGI process " << pid << " exited with code " 
                              << exitCode << std::endl;
                }
            }
            
            // Limpar
            int clientFd = process->getClientFd();
            _clientToProcess.erase(clientFd);
            delete process;
            _activeProcesses.erase(it);
        }
    }
}

void CgiHandler::checkTimeouts() {
    std::map<pid_t, CgiProcess*>::iterator it = _activeProcesses.begin();
    
    while (it != _activeProcesses.end()) {
        CgiProcess* process = it->second;
        
        if (process->isTimedOut()) {
            std::cout << "CGI process " << process->getPid() << " timed out" << std::endl;
            process->killProcess();
            
            // Limpar mapeamento
            int clientFd = process->getClientFd();
            _clientToProcess.erase(clientFd);
            
            // Enviar erro 504 ao cliente se possível
            if (clientFd != -1) {
                try {
                    Client& client = RunTime::getClient(clientFd);
                    client.getResponse().setStatus(504, "Gateway Timeout");
                    client.getResponse().setBody("<h1>504 Gateway Timeout</h1>", "text/html");
                } catch (...) {
                    // Cliente não existe mais
                }
            }
            
            // Limpar processo (C++98: erase não retorna iterator)
            pid_t pidToErase = it->first;
            delete process;
            ++it; // Avançar antes de deletar
            _activeProcesses.erase(pidToErase);
        } else {
            ++it;
        }
    }
}

void CgiHandler::checkPendingProcesses() {
    // Verificar processos com entrada fechada (POST) que podem ter gerado saída
    if (_activeProcesses.empty()) {
        return; // Sem processos ativos
    }
    
    std::map<pid_t, CgiProcess*>::iterator it = _activeProcesses.begin();
    
    int pendingCount = 0;
    int totalProcesses = 0;
    while (it != _activeProcesses.end()) {
        CgiProcess* process = it->second;
        totalProcesses++;
        
        bool inputClosed = process->isInputClosed();
        bool finished = process->isFinished();
        int pipeOutFd = process->getPipeOutFd();
        
        // Se entrada está fechada mas processo ainda não terminou
        if (inputClosed && !finished && pipeOutFd != -1) {
            pendingCount++;
            std::cout << "CGI: Checking pending process " << process->getPid() 
                      << " (input closed, not finished, pipeOut=" << pipeOutFd << ")" << std::endl;
            
            // Tentar ler do pipe de saída (non-blocking)
            process->readFromStdout();
            
            // Verificar se processo terminou após ler
            if (process->isFinished()) {
                int clientFd = process->getClientFd();
                
                try {
                    Client& client = RunTime::getClient(clientFd);
                    
                    std::string cgiOutput = process->getOutput();
                    if (!cgiOutput.empty()) {
                        std::cout << "CGI: Got response from pending process " << process->getPid() << std::endl;
                        client.getResponse().processCgiResponse(cgiOutput);
                        std::string responseStr = client.getResponse().toString();
                        cleanupClientProcess(clientFd);
                        client.sendResponse(responseStr);
                    }
                } catch (...) {
                    // Cliente não existe mais - limpar processo
                    cleanupProcess(process->getPid());
                }
            } else {
                // Verificar se processo realmente terminou (pode ter terminado mas pipe ainda não fechou)
                int status;
                pid_t waitResult = waitpid(process->getPid(), &status, WNOHANG);
                if (waitResult == process->getPid()) {
                    // Processo terminou - tentar ler dados restantes
                    process->readFromStdout();
                    
                    int clientFd = process->getClientFd();
                    try {
                        Client& client = RunTime::getClient(clientFd);
                        
                        std::string cgiOutput = process->getOutput();
                        if (!cgiOutput.empty()) {
                            std::cout << "CGI: Process terminated, got response from pending process" << std::endl;
                            client.getResponse().processCgiResponse(cgiOutput);
                            std::string responseStr = client.getResponse().toString();
                            cleanupClientProcess(clientFd);
                            client.sendResponse(responseStr);
                        }
                    } catch (...) {
                        cleanupProcess(process->getPid());
                    }
                }
            }
        }
        
        ++it;
    }
    
    if (pendingCount > 0) {
        std::cout << "CGI: Found " << pendingCount << " pending processes" << std::endl;
    }
}

CgiProcess* CgiHandler::getProcessByClient(int clientFd) {
    std::map<int, CgiProcess*>::iterator it = _clientToProcess.find(clientFd);
    if (it != _clientToProcess.end()) {
        return it->second;
    }
    return NULL;
}

bool CgiHandler::hasActiveProcess(int clientFd) {
    return _clientToProcess.find(clientFd) != _clientToProcess.end();
}


#pragma once

#include "WebservHeader.hpp"

class HttpRequest;
class ServerBlock;
class LocationBlock;
class Client;
class CgiPipeHandler;
class CgiProcess;

class CgiHandler {
private:
    static std::map<pid_t, CgiProcess*> _activeProcesses;  // Rastreamento de processos
    static std::map<int, CgiProcess*> _clientToProcess;    // Mapeamento cliente → processo
    static std::map<int, CgiPipeHandler*> _pipeHandlers;    // Mapeamento pipe FD → handler
    
    // Helpers privados
    static std::string getCgiScriptPath(const std::string& uri, 
                                         const ServerBlock& serverBlock,
                                         const LocationBlock& location);
    static std::vector<std::string> buildEnvironment(const HttpRequest& req,
                                                       const ServerBlock& serverBlock,
                                                       const LocationBlock& location,
                                                       const std::string& scriptPath);
    static std::string normalizeHeaderName(const std::string& header);
    static std::string extractScriptName(const std::string& uri);
    static std::string extractPathInfo(const std::string& uri);
    static std::string extractQueryString(const std::string& uri);
    static std::string intToString(int n);
    
public:
    // Verificação se precisa executar CGI
    static bool shouldExecuteCgi(const std::string& uri, 
                                  const LocationBlock& location);
    
    // Execução CGI síncrona (para requisições pequenas)
    static bool executeCgi(const HttpRequest& req,
                           const ServerBlock& serverBlock,
                           const LocationBlock& location,
                           std::string& output);
    
    // Execução CGI assíncrona (para requisições grandes ou assíncronas)
    static bool executeCgiAsync(const HttpRequest& req,
                                 const ServerBlock& serverBlock,
                                 const LocationBlock& location,
                                 int clientFd);
    
    // Callbacks para epoll
    static void handleCgiPipeIn(int fd, Client* client);
    static void handleCgiPipeOut(int fd, Client* client);
    
    // Limpeza
    static void cleanupProcess(pid_t pid);
    static void cleanupClientProcess(int clientFd);
    static void cleanupPipeHandler(int pipeFd);
    static void cleanupZombieProcesses();
    static void checkTimeouts();
    static void checkPendingProcesses(); // Verificar processos com entrada fechada tentando ler saída
    
    // Getters
    static CgiProcess* getProcessByClient(int clientFd);
    static bool hasActiveProcess(int clientFd);
    
    // Helper para encontrar melhor location match
    static std::string findBestLocationMatch(const std::string& uri,
                                              const ServerBlock& serverBlock,
                                              LocationBlock& location);
};


# Implementação CGI - WebServ

## Visão Geral

Este documento descreve a implementação completa do sistema CGI (Common Gateway Interface) para o WebServ, incluindo comunicação bidirecional via pipes, processamento assíncrono, proteção contra processos travados e gerenciamento adequado de memória.

---

## 1. Arquitetura Geral

### 1.1 Fluxo de Execução CGI

```
Cliente → HttpRequest → HttpResponse → CgiHandler → Processo CGI → Resposta → Cliente
            ↓              ↓              ↓              ↓              ↓
         Parser        Detecta CGI    Cria Pipes    Executa Script   Envia HTTP
```

### 1.2 Componentes Principais

- **`CgiHandler`**: Classe principal que gerencia execução CGI
- **`CgiProcess`**: Representa um processo CGI ativo
- **`CgiPipeHandler`**: Handler de epoll para pipes CGI (herda de `EpollHandler`)
- **Integração com `HttpResponse`**: Detecta quando URI precisa de CGI
- **Integração com `Client`**: Gerencia I/O assíncrono via epoll através de `CgiPipeHandler`

---

## 2. Estrutura de Classes

### 2.1 Classe `CgiProcess`

**Responsabilidades:**
- Gerenciar processo filho executando script CGI
- Controlar pipes de entrada/saída
- Rastrear timeout e estado do processo
- Limpar recursos ao finalizar

**Membros Privados:**
```cpp
class CgiProcess {
private:
    pid_t           _pid;              // PID do processo filho
    int             _pipeIn[2];        // Pipe para enviar dados ao CGI
    int             _pipeOut[2];       // Pipe para receber dados do CGI
    time_t          _startTime;        // Timestamp de início
    int             _timeoutSeconds;    // Timeout (padrão: 2s)
    bool            _finished;         // Flag se processo terminou
    bool            _inputClosed;       // Flag se pipe de entrada foi fechado
    std::string     _outputBuffer;     // Buffer acumulado da resposta CGI
    int             _clientFd;         // Socket do cliente (para epoll)
    std::string     _scriptPath;       // Caminho do script CGI
    std::string     _remainingBody;    // Body que ainda precisa ser enviado
    std::vector<std::string> _env;     // Variáveis de ambiente
    
    void setupChildProcess();
    void setupParentProcess();
    std::string getInterpreter(const std::string& scriptPath) const;
    
    // Construtor de cópia e operador de atribuição desabilitados
    // Razão: CgiProcess gerencia recursos únicos (PID, pipes, file descriptors)
    // que não podem ser copiados semânticamente. Cada instância representa
    // um processo CGI único e não deve ser duplicada.
    CgiProcess(const CgiProcess& other);
    CgiProcess& operator=(const CgiProcess& other);
    
public:
    CgiProcess(const std::string& scriptPath, 
               const std::vector<std::string>& env,
               int clientFd);
    ~CgiProcess();
    
    // Gerenciamento de processo
    bool start();
    bool isFinished() const;
    bool isTimedOut() const;
    void killProcess();
    
    // I/O assíncrono
    // Retorna true se escreveu tudo, false se escreveu parcialmente ou não escreveu
    // Atualiza _remainingBody automaticamente se escreveu parcialmente
    bool writeToStdin(const std::string& data);
    bool readFromStdout();
    std::string getOutput() const;
    bool isInputClosed() const;
    
    // Getters
    int getPipeInFd() const;
    int getPipeOutFd() const;
    pid_t getPid() const;
    int getClientFd() const;
    std::string getRemainingBody() const;
    void setRemainingBody(const std::string& body);
    void closeInputPipe(); // Fechar pipe de entrada (útil para GET sem body)
};
```

### 2.2 Classe `CgiHandler`

**Responsabilidades:**
- Detectar se URI precisa de CGI
- Criar e gerenciar processos CGI
- Preparar variáveis de ambiente
- Limpar processos zumbis
- Integrar com sistema de epoll

**Membros Privados:**
```cpp
class CgiHandler {
private:
    static std::map<pid_t, CgiProcess*> _activeProcesses;  // Rastreamento de processos
    static std::map<int, CgiProcess*> _clientToProcess;     // Mapeamento cliente → processo
    static std::map<int, CgiPipeHandler*> _pipeHandlers;   // Mapeamento pipe FD → handler
    
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
```

### 2.3 Classe `CgiPipeHandler`

**Responsabilidades:**
- Herda de `EpollHandler` para gerenciar eventos de epoll nos pipes CGI
- Encapsula lógica de I/O assíncrono para pipes de entrada/saída
- Delega chamadas para `CgiHandler::handleCgiPipeIn/Out`

**Membros:**
```cpp
class CgiPipeHandler : public EpollHandler {
private:
    int _clientFd;          // FD do cliente relacionado
    bool _isInputPipe;      // true se é pipe de entrada, false se é pipe de saída
    
public:
    CgiPipeHandler(int pipeFd, int clientFd, bool isInputPipe);
    virtual ~CgiPipeHandler();
    
    virtual void handleEpollIn(void);
    virtual void handleEpollOut(void);
    
    int getClientFd() const;
    bool isInputPipe() const;
};
```

---

## 3. Detecção de CGI

### 3.1 Algoritmo de Detecção

1. **Verificar extensão do arquivo:**
   - Extrair extensão do URI (ex: `/script.py` → `.py`)
   - Comparar com `location.getCgiExtensions()` (ex: `[".py", ".pl", ".php"]`)
   - Se match, precisa executar CGI

2. **Verificar se arquivo existe:**
   - Construir caminho completo: `root + uri` ou `alias + uri`
   - Verificar se arquivo existe e é executável

3. **Verificar método HTTP:**
   - CGI geralmente suporta GET e POST
   - Validar contra `location.getAllowMethods()`

**Implementação:**
```cpp
bool CgiHandler::shouldExecuteCgi(const std::string& uri, 
                                   const LocationBlock& location) {
    // Extrair extensão
    size_t dotPos = uri.find_last_of('.');
    if (dotPos == std::string::npos)
        return false;
    
    std::string extension = uri.substr(dotPos);
    std::vector<std::string> cgiExtensions = location.getCgiExtensions();
    
    // Verificar se extensão está na lista
    for (size_t i = 0; i < cgiExtensions.size(); i++) {
        if (cgiExtensions[i] == extension)
            return true;
    }
    
    return false;
}
```

---

## 4. Comunicação Bidirecional via Pipes

### 4.1 Criação de Pipes

```cpp
bool CgiProcess::start() {
    // Criar pipes
    if (pipe(_pipeIn) == -1 || pipe(_pipeOut) == -1) {
        return false;
    }
    
    // Tornar pipes non-blocking
    fcntl(_pipeIn[1], F_SETFL, O_NONBLOCK);
    fcntl(_pipeOut[0], F_SETFL, O_NONBLOCK);
    
    // Fork
    _pid = fork();
    if (_pid == -1) {
        close(_pipeIn[0]);
        close(_pipeIn[1]);
        close(_pipeOut[0]);
        close(_pipeOut[1]);
        return false;
    }
    
    if (_pid == 0) {
        // Processo filho
        setupChildProcess();
        
        // Obter interpretador baseado na extensão
        std::string interpreter = getInterpreter(_scriptPath);
        
        // Preparar argv: [interpreter, script_path, NULL]
        char* argv[3];
        argv[0] = const_cast<char*>(interpreter.c_str());
        argv[1] = const_cast<char*>(_scriptPath.c_str());
        argv[2] = NULL;
        
        // Converter vector<string> para char*[]
        char** envp = new char*[_env.size() + 1];
        for (size_t i = 0; i < _env.size(); i++) {
            envp[i] = const_cast<char*>(_env[i].c_str());
        }
        envp[_env.size()] = NULL;
        
        execve(interpreter.c_str(), argv, envp);
        
        // Se chegou aqui, execve falhou
        std::cerr << "CGI: execve failed: " << strerror(errno) << std::endl;
        delete[] envp;
        exit(1);
    } else {
        // Processo pai
        setupParentProcess();
        _startTime = time(NULL);
    }
    
    return true;
}
```

### 4.2 Setup do Processo Filho

```cpp
void CgiProcess::setupChildProcess() {
    // Redirecionar stdin para pipe de entrada
    dup2(_pipeIn[0], STDIN_FILENO);
    
    // Redirecionar stdout para pipe de saída
    dup2(_pipeOut[1], STDOUT_FILENO);
    
    // Fechar pipes desnecessários
    close(_pipeIn[0]);
    close(_pipeIn[1]);
    close(_pipeOut[0]);
    close(_pipeOut[1]);
    
    // Fechar outros file descriptors (exceto stdin, stdout, stderr)
    // Limitar file descriptors abertos para segurança
    for (int i = 3; i < 1024; i++) {
        close(i);
    }
}
```

### 4.3 Setup do Processo Pai

```cpp
void CgiProcess::setupParentProcess() {
    // Fechar extremidades não usadas
    close(_pipeIn[0]);   // Não lemos do pipe de entrada
    close(_pipeOut[1]);  // Não escrevemos no pipe de saída
    
    // NOTA: O registro com epoll é feito em CgiHandler::executeCgiAsync()
    // através de CgiPipeHandler, não diretamente aqui
}
```

---

## 5. Processamento Assíncrono

### 5.1 Escrita Incremental do Body

**Problema:** Body pode ser grande (ex: 100MB upload)
**Solução:** Escrever incrementalmente quando pipe estiver pronto para escrita

```cpp
bool CgiProcess::writeToStdin(const std::string& data) {
    if (_inputClosed || _pipeIn[1] == -1)
        return false;
    
    ssize_t written = write(_pipeIn[1], data.c_str(), data.size());
    
    if (written < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Pipe não está pronto - esperar próximo evento EPOLLOUT
            return false;
        }
        // Erro real - fechar pipe
        close(_pipeIn[1]);
        _pipeIn[1] = -1;
        _inputClosed = true;
        return false;
    }
    
    // Se escreveu tudo, fechar pipe
    if (static_cast<size_t>(written) == data.size()) {
        // IMPORTANTE: Quando fechamos o pipe de escrita no processo pai,
        // o processo filho (CGI) deve receber EOF no stdin
        close(_pipeIn[1]);
        _pipeIn[1] = -1;
        _inputClosed = true;
        
        // Pequeno delay para garantir que o EOF seja propagado
        usleep(10000); // 10ms
        
        return true;
    }
    
    // Escreveu parcialmente - atualizar remainingBody com o que sobrou
    _remainingBody = data.substr(written);
    return false;
}
```

### 5.2 Leitura Incremental da Resposta

```cpp
bool CgiProcess::readFromStdout() {
    if (_finished || _pipeOut[0] == -1)
        return false;
    
    char buffer[8192];
    ssize_t bytesRead = read(_pipeOut[0], buffer, sizeof(buffer));
    
    if (bytesRead > 0) {
        _outputBuffer.append(buffer, bytesRead);
        return true;
    } else if (bytesRead == 0) {
        // EOF - processo terminou de escrever
        close(_pipeOut[0]);
        _pipeOut[0] = -1;
        
        // Quando recebemos EOF, o processo já terminou (ou está para terminar)
        // Marcar como finished para processar resposta
        _finished = true;
        
        // Tentar coletar o processo (não bloqueante)
        int status;
        pid_t waitResult = waitpid(_pid, &status, WNOHANG);
        if (waitResult == _pid) {
            // Processo terminou
        }
        return false;
    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Nada para ler agora
            return true;
        }
        // Erro real
        close(_pipeOut[0]);
        _pipeOut[0] = -1;
        _finished = true;
        return false;
    }
}
```

### 5.3 Integração com Epoll via CgiPipeHandler

**Criação de Handlers em `executeCgiAsync()`:**
```cpp
// Criar handlers para os pipes e adicionar ao epoll
EpollInstance& epoll = RunTime::getEpoll();

// Handler para pipe de entrada (escrita) - apenas se houver body E pipe ainda aberto
if (process->getPipeInFd() != -1 && !req.getBody().empty()) {
    CgiPipeHandler* inputHandler = new CgiPipeHandler(process->getPipeInFd(), clientFd, true);
    _pipeHandlers[process->getPipeInFd()] = inputHandler;
    epoll.manipInterestList(EPOLL_CTL_ADD, inputHandler);
    
    // Adicionar evento EPOLLOUT imediatamente pois há body para enviar
    inputHandler->setInterestedEvents(EPOLLOUT);
    epoll.manipInterestList(EPOLL_CTL_MOD, inputHandler);
}

// Handler para pipe de saída (leitura) - sempre necessário
if (process->getPipeOutFd() != -1) {
    CgiPipeHandler* outputHandler = new CgiPipeHandler(process->getPipeOutFd(), clientFd, false);
    _pipeHandlers[process->getPipeOutFd()] = outputHandler;
    epoll.manipInterestList(EPOLL_CTL_ADD, outputHandler);
}
```

**Implementação de `CgiPipeHandler::handleEpollIn()`:**
```cpp
void CgiPipeHandler::handleEpollIn(void) {
    if (!_isInputPipe) {
        // É pipe de saída - ler dados do CGI
        try {
            Client& client = RunTime::getClient(_clientFd);
            CgiHandler::handleCgiPipeOut(this->getSocketFd(), &client);
        } catch (...) {
            // Cliente não existe mais - limpar processo
            CgiHandler::cleanupClientProcess(_clientFd);
        }
    }
}
```

**Implementação de `CgiPipeHandler::handleEpollOut()`:**
```cpp
void CgiPipeHandler::handleEpollOut(void) {
    if (_isInputPipe) {
        // É pipe de entrada - escrever dados no CGI
        try {
            Client& client = RunTime::getClient(_clientFd);
            CgiHandler::handleCgiPipeIn(this->getSocketFd(), &client);
        } catch (...) {
            // Cliente não existe mais - limpar processo
            CgiHandler::cleanupClientProcess(_clientFd);
        }
    }
}
```

**No loop principal (`main.cpp`):**
```cpp
void serverMainLoop() {
    while (true) {
        int numberOfReadySockets = RunTime::getEpoll().manipEpollWait();
        if (numberOfReadySockets == -1) {
            std::cerr << "Error: erro ao manipular o epoll_wait()." << std::endl;
            break;
        } else {
            epollReadyListLoop(numberOfReadySockets);
            
            // Limpar processos zumbis CGI
            CgiHandler::cleanupZombieProcesses();
            
            // Verificar processos CGI pendentes (entrada fechada mas ainda rodando)
            CgiHandler::checkPendingProcesses();
            
            // Verificar timeouts de processos CGI (2 segundos)
            CgiHandler::checkTimeouts();
        }
    }
}
```

---

## 6. Variáveis de Ambiente CGI

### 6.1 Variáveis Obrigatórias

```cpp
std::vector<std::string> CgiHandler::buildEnvironment(
    const HttpRequest& req,
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
```

### 6.2 Conversão de Headers

```cpp
std::string normalizeHeaderName(const std::string& header) {
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
```

---

## 7. Proteção contra Processos Travados

### 7.1 Timeout de 2 Segundos

```cpp
bool CgiProcess::isTimedOut() const {
    if (_finished)
        return false;
    
    time_t currentTime = time(NULL);
    return (currentTime - _startTime) >= _timeoutSeconds;
}
```

### 7.2 Verificação Periódica

**No loop principal (`main.cpp`):**
```cpp
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
```

### 7.5 Verificação de Processos Pendentes

**Método `checkPendingProcesses()`:**
```cpp
void CgiHandler::checkPendingProcesses() {
    // Verificar processos com entrada fechada (POST) que podem ter gerado saída
    if (_activeProcesses.empty()) {
        return; // Sem processos ativos
    }
    
    std::map<pid_t, CgiProcess*>::iterator it = _activeProcesses.begin();
    
    while (it != _activeProcesses.end()) {
        CgiProcess* process = it->second;
        
        bool inputClosed = process->isInputClosed();
        bool finished = process->isFinished();
        int pipeOutFd = process->getPipeOutFd();
        
        // Se entrada está fechada mas processo ainda não terminou
        if (inputClosed && !finished && pipeOutFd != -1) {
            // Tentar ler do pipe de saída (non-blocking)
            process->readFromStdout();
            
            // Verificar se processo terminou após ler
            if (process->isFinished()) {
                int clientFd = process->getClientFd();
                try {
                    Client& client = RunTime::getClient(clientFd);
                    std::string cgiOutput = process->getOutput();
                    if (!cgiOutput.empty()) {
                        client.getResponse().processCgiResponse(cgiOutput);
                        std::string responseStr = client.getResponse().toString();
                        cleanupClientProcess(clientFd);
                        client.sendResponse(responseStr);
                    }
                } catch (...) {
                    // Cliente não existe mais - limpar processo
                    cleanupProcess(process->getPid());
                }
            }
        }
        
        ++it;
    }
}
```

### 7.3 Kill Process

```cpp
void CgiProcess::killProcess() {
    if (_pid > 0 && !_finished) {
        kill(_pid, SIGTERM);
        
        // Esperar um pouco
        usleep(100000); // 100ms
        
        // Se ainda não terminou, forçar
        if (!isFinished()) {
            kill(_pid, SIGKILL);
        }
        
        // Limpar pipes
        if (!_inputClosed && _pipeIn[1] != -1) {
            close(_pipeIn[1]);
            _pipeIn[1] = -1;
            _inputClosed = true;
        }
        if (_pipeOut[0] != -1) {
            close(_pipeOut[0]);
            _pipeOut[0] = -1;
        }
        
        _finished = true;
    }
}
```

### 7.4 Limpeza de Processos Zumbis

```cpp
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
```

**Chamar periodicamente:**
```cpp
// No loop principal do servidor (main.cpp)
void serverMainLoop() {
    while (true) {
        int numberOfReadySockets = RunTime::getEpoll().manipEpollWait();
        if (numberOfReadySockets == -1) {
            std::cerr << "Error: erro ao manipular o epoll_wait()." << std::endl;
            break;
        } else {
            epollReadyListLoop(numberOfReadySockets);
            
            // Limpar processos zumbis periodicamente
            CgiHandler::cleanupZombieProcesses();
            
            // Verificar processos pendentes
            CgiHandler::checkPendingProcesses();
            
            // Verificar timeouts
            CgiHandler::checkTimeouts();
        }
    }
}
```

---

## 8. Processamento de Resposta CGI

### 8.1 Parsing da Saída CGI

**Formato de saída CGI:**
```
Headers HTTP\r\n
\r\n
Body
```

**Implementação:**
```cpp
void HttpResponse::processCgiResponse(const std::string& cgiOutput) {
    size_t headerEnd = cgiOutput.find("\r\n\r\n");
    
    if (headerEnd == std::string::npos) {
        // Não encontrou separador - tratar como erro
        this->setStatus(502, "Bad Gateway");
        this->setBody("<h1>502 Bad Gateway</h1>", "text/html");
        return;
    }
    
    // Extrair headers
    std::string headersStr = cgiOutput.substr(0, headerEnd);
    std::string body = cgiOutput.substr(headerEnd + 4);
    
    // Parsear headers
    std::istringstream headerStream(headersStr);
    std::string line;
    
    while (std::getline(headerStream, line)) {
        if (line.empty() || line == "\r")
            break;
        
        // Remover \r
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // Remover espaços do início do value
            while (!value.empty() && value[0] == ' ')
                value.erase(0, 1);
            
            // Status especial (comparação case-insensitive)
            std::string lowerKey = key;
            for (size_t i = 0; i < lowerKey.size(); i++) {
                lowerKey[i] = std::tolower(lowerKey[i]);
            }
            
            if (lowerKey == "status") {
                int statusCode = std::atoi(value.substr(0, 3).c_str());
                std::string statusMsg = value.substr(4);
                this->setStatus(statusCode, statusMsg);
            } else {
                this->setHeader(key, value);
            }
        }
    }
    
    // Se não há status code nos headers, usar padrão
    if (this->_status_code == 200) {
        this->setStatus(200, "OK");
    }
    
    // Adicionar body
    std::string contentType = this->getHeaderValue("Content-Type");
    if (contentType.empty()) {
        contentType = "text/html";
    }
    this->setBody(body, contentType);
}
```

---

## 9. Integração com HttpResponse

### 9.1 Modificação em `dispatchRequest()`

```cpp
void HttpResponse::dispatchRequest(const HttpRequest &req) {
    if (this->_status_code != 200)
        return;
    
    // Encontrar location block correspondente
    ServerBlock serverBlock = // ... obter do contexto ...
    std::string uri = req.getUri();
    
    // Encontrar melhor match de location
    std::string bestMatch = findBestLocationMatch(uri, serverBlock);
    LocationBlock location = serverBlock.getLocations()[bestMatch];
    
    // Verificar se precisa executar CGI
    if (CgiHandler::shouldExecuteCgi(uri, location)) {
        // Executar CGI
        std::string cgiOutput;
        
        if (CgiHandler::executeCgi(req, serverBlock, location, cgiOutput)) {
            processCgiResponse(cgiOutput);
        } else {
            setStatus(502, "Bad Gateway");
            setBody("<h1>502 Bad Gateway</h1>", "text/html");
        }
        return;
    }
    
    // Processamento normal
    if (req.getMethod() == "GET")
        return handleGet(req);
    else if (req.getMethod() == "POST")
        return handlePost(req);
    else if (req.getMethod() == "DELETE")
        return handleDelete(req);
    else 
        this->setErrorPage(405);
}
```

### 9.2 Versão Assíncrona

Para requisições grandes, usar versão assíncrona:

```cpp
bool HttpResponse::dispatchRequestAsync(const HttpRequest &req, 
                                        const ServerBlock &serverBlock,
                                        int clientFd) {
    if (this->_status_code != 200)
        return true;
    
    // Encontrar melhor match de location
    std::map<std::string, LocationBlock> locations = serverBlock.getLocations();
    std::string bestMatch = "";
    const LocationBlock* locationPtr = NULL;
    
    for (std::map<std::string, LocationBlock>::const_iterator it = locations.begin();
         it != locations.end(); ++it) {
        const std::string &path = it->first;
        if (req.getUri().compare(0, path.size(), path) == 0) {
            if (path.size() > bestMatch.size()) {
                bestMatch = path;
                locationPtr = &(it->second);
            }
        }
    }
    
    // Verificar se precisa executar CGI
    if (!bestMatch.empty() && locationPtr && 
        CgiHandler::shouldExecuteCgi(req.getUri(), *locationPtr)) {
        // Executar CGI assíncrono
        if (CgiHandler::executeCgiAsync(req, serverBlock, *locationPtr, clientFd)) {
            // Requisição será processada via callbacks de epoll (CgiPipeHandler)
            return false; // Indica que resposta ainda não está pronta
        } else {
            setStatus(502, "Bad Gateway");
            setBody("<h1>502 Bad Gateway</h1>", "text/html");
            return true; // Resposta pronta (erro)
        }
    }
    
    // Processamento normal (síncrono)
    dispatchRequest(req, serverBlock);
    return true; // Resposta pronta
}
```

---

## 10. Configuração

### 10.1 Exemplo de Config File

```nginx
server {
    listen 8080;
    server_name localhost;
    root /var/www;
    client_max_body_size 100M;
    
    location /cgi-bin {
        alias /var/www/cgi-bin;
        cgi_extensions .py .php;  # NOTA: Apenas .py e .php são aceitos pelo parser
        allow_methods GET POST;
        cookies_enabled true;
    }
    
    location / {
        allow_methods GET POST DELETE;
        autoindex false;
    }
}
```

**Extensões Suportadas:**
- **`.py`** → Python 3 (`/usr/bin/python3`)
- **`.php`** → PHP (`/usr/bin/php`)

**NOTA:** Embora o código de execução (`CgiProcess::getInterpreter()`) tenha suporte para `.pl` (Perl) e `.sh` (Bash), o parser de configuração (`LocationBlock::addCgiExtensions()`) atualmente só aceita `.php` e `.py`. Tentar configurar outras extensões resultará em erro de parsing.

### 10.2 Validação de Configuração

- Verificar se extensões CGI são válidas
- Verificar se caminho do script existe
- Verificar se script é executável

---

## 11. Tratamento de Erros

### 11.1 Erros Comuns

1. **502 Bad Gateway**: Script CGI não executou corretamente
2. **500 Internal Server Error**: Erro ao criar pipes/fork
3. **504 Gateway Timeout**: Script CGI demorou mais de 2s
4. **403 Forbidden**: Script não é executável

### 11.2 Implementação

```cpp
void HttpResponse::handleCgiError(int errorCode, const std::string& message) {
    switch (errorCode) {
        case CGI_ERROR_FORK:
        case CGI_ERROR_PIPE:
            setStatus(500, "Internal Server Error");
            break;
        case CGI_ERROR_TIMEOUT:
            setStatus(504, "Gateway Timeout");
            break;
        case CGI_ERROR_EXEC:
            setStatus(502, "Bad Gateway");
            break;
        default:
            setStatus(500, "Internal Server Error");
    }
    
    setBody("<h1>" + std::to_string(getStatusCode()) + " " 
            + getStatusMessage() + "</h1><p>" + message + "</p>", 
            "text/html");
}
```

---

## 12. Segurança

### 12.1 Validações Importantes

1. **Validar caminho do script:**
   - Não permitir `../` (path traversal)
   - Verificar se está dentro do root permitido
   - Verificar se arquivo existe

2. **Limitar recursos:**
   - Timeout de 2s
   - Limitar tamanho de body
   - Limitar número de processos CGI simultâneos

3. **Sanitizar variáveis de ambiente:**
   - Não passar dados sensíveis
   - Validar valores antes de passar

---

## 13. Fluxo Completo Assíncrono

### 13.1 Exemplo de Requisição POST para CGI

```
1. Cliente envia requisição POST /cgi-bin/script.py
2. Client::handleEpollIn() recebe dados do socket do cliente
3. HttpRequest parseia requisição
4. HttpResponse::dispatchRequestAsync() detecta CGI
5. CgiHandler::executeCgiAsync() cria processo
6. CgiProcess cria pipes e faz fork
7. CgiHandler cria CgiPipeHandler para cada pipe e registra no epoll
8. Processo filho executa script.py (via execve com interpretador)
9. CgiPipeHandler::handleEpollOut() é chamado quando pipeIn está pronto
10. CgiHandler::handleCgiPipeIn() escreve body no pipeIn incrementalmente
11. Quando body completo é enviado, pipe de entrada é fechado
12. Script CGI processa e escreve resposta no stdout
13. CgiPipeHandler::handleEpollIn() é chamado quando pipeOut tem dados
14. CgiHandler::handleCgiPipeOut() lê resposta do pipeOut incrementalmente
15. Quando processo termina (EOF), HttpResponse::processCgiResponse() parseia resposta
16. Client::sendResponse() envia resposta HTTP ao cliente
17. CgiHandler::cleanupClientProcess() limpa recursos (processo, handlers, pipes)
```

### 13.2 Gerenciamento de Body Incremental

**Escrita incremental com `_remainingBody`:**
```cpp
// Em executeCgiAsync, armazenar body completo
if (!req.getBody().empty()) {
    process->setRemainingBody(req.getBody());
}

// Em handleCgiPipeIn, escrever incrementalmente
std::string body = process->getRemainingBody();
if (!body.empty()) {
    bool wroteAll = process->writeToStdin(body);
    
    if (wroteAll) {
        // Tudo foi escrito - limpar remaining body
        process->setRemainingBody("");
        // Pipe foi fechado automaticamente por writeToStdin
    }
    // Se escreveu parcialmente, writeToStdin atualiza _remainingBody automaticamente
}
```

### 13.3 Detecção de Interpretador

**Método `getInterpreter()`:**
```cpp
std::string CgiProcess::getInterpreter(const std::string& scriptPath) const {
    // Extrair extensão
    size_t dotPos = scriptPath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return scriptPath; // Sem extensão - tentar executar diretamente
    }
    
    std::string extension = scriptPath.substr(dotPos);
    
    // Determinar interpretador baseado na extensão
    if (extension == ".py") {
        return "/usr/bin/python3";
    } else if (extension == ".pl") {
        return "/usr/bin/perl";
    } else if (extension == ".sh") {
        return "/bin/bash";
    } else if (extension == ".php") {
        return "/usr/bin/php";
    } else {
        return scriptPath; // Tentar executar diretamente
    }
}
```

**NOTA IMPORTANTE:** Embora o método `getInterpreter()` suporte `.py`, `.pl`, `.sh` e `.php`, o parser de configuração em `LocationBlock::addCgiExtensions()` atualmente **só aceita `.php` e `.py`** no arquivo de configuração. Portanto, na prática, apenas essas duas extensões são suportadas pelo sistema completo. Tentar configurar `.pl` ou `.sh` no arquivo de configuração resultará em erro de parsing.

---

## 14. Checklist de Implementação

### Fase 1: Estrutura Básica ✅
- [x] Criar classe `CgiProcess`
- [x] Criar classe `CgiHandler`
- [x] Criar classe `CgiPipeHandler`
- [x] Implementar detecção de CGI
- [x] Implementar criação de pipes

### Fase 2: Execução Básica ✅
- [x] Implementar fork/execve
- [x] Implementar detecção de interpretador
- [x] Implementar variáveis de ambiente
- [x] Implementar escrita/leitura básica de pipes
- [x] Testar com script simples

### Fase 3: Assíncrono ✅
- [x] Integrar com epoll via `CgiPipeHandler`
- [x] Implementar escrita incremental com `_remainingBody`
- [x] Implementar leitura incremental
- [x] Implementar fechamento automático de pipe de entrada para GET
- [x] Testar com body grande

### Fase 4: Proteção e Limpeza ✅
- [x] Implementar timeout de 2 segundos
- [x] Implementar kill de processos (SIGTERM + SIGKILL)
- [x] Implementar limpeza de zumbis
- [x] Implementar `checkPendingProcesses()` para processos com entrada fechada
- [x] Implementar limpeza de `CgiPipeHandler`
- [x] Testar com script que trava

### Fase 5: Parsing e Validação ✅
- [x] Implementar parsing de resposta CGI
- [x] Implementar comparação case-insensitive para header "Status"
- [x] Implementar validações de segurança (path traversal, arquivo existe, é regular)
- [x] Testes completos

---

## 15. Exemplo de Script CGI de Teste

**test.py:**
```python
#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html\r")
print("Status: 200 OK\r")
print("\r")

print("<html><body>")
print("<h1>CGI Test</h1>")
print("<p>REQUEST_METHOD:", os.environ.get('REQUEST_METHOD', ''), "</p>")
print("<p>QUERY_STRING:", os.environ.get('QUERY_STRING', ''), "</p>")

# Ler body se POST
if os.environ.get('REQUEST_METHOD') == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        body = sys.stdin.read(content_length)
        print("<p>Body:", body[:100], "</p>")

print("</body></html>")
```

**Tornar executável:**
```bash
chmod +x test.py
```

---

## 16. Considerações Finais

### 16.1 Performance

- Use buffers adequados (8192 bytes para leitura)
- Limite número de processos CGI simultâneos
- Reuse processos quando possível (opcional, avançado)

### 16.2 Debugging

- Logs detalhados de cada etapa
- Verificar estado dos pipes
- Verificar se processos estão sendo limpos

### 16.3 Testes

- Script simples que retorna HTML
- Script que processa POST com body grande
- Script que demora mais de 2s (timeout)
- Script que retorna erro 500

---

## Conclusão

Esta implementação fornece um sistema CGI robusto, assíncrono e seguro, integrado com o sistema de I/O assíncrono existente do WebServ. O uso de pipes non-blocking e epoll garante que o servidor não seja bloqueado durante execução de scripts CGI.


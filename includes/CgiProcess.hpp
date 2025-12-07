#pragma once

#include "WebservHeader.hpp"

class HttpRequest;
class ServerBlock;
class LocationBlock;

class CgiProcess {
private:
    pid_t           _pid;              // PID do processo filho
    int             _pipeIn[2];        // Pipe para enviar dados ao CGI
    int             _pipeOut[2];       // Pipe para receber dados do CGI
    time_t          _startTime;        // Timestamp de início
    int             _timeoutSeconds;   // Timeout (padrão: 2s)
    bool            _finished;          // Flag se processo terminou
    bool            _inputClosed;      // Flag se pipe de entrada foi fechado
    std::string     _outputBuffer;     // Buffer acumulado da resposta CGI
    int             _clientFd;         // Socket do cliente (para epoll)
    std::string     _scriptPath;       // Caminho do script CGI
    std::string     _remainingBody;     // Body que ainda precisa ser enviado
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
    CgiProcess(const std::string& scriptPath, const std::vector<std::string>& env,
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


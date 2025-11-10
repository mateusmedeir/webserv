#include "../includes/CgiProcess.hpp"
#include "../includes/CgiHandler.hpp"
#include "../includes/RunTime.hpp"
#include <sys/wait.h>
#include <signal.h>
#include <cstdlib>
#include <cstring>

CgiProcess::CgiProcess(const std::string& scriptPath, 
                       const std::vector<std::string>& env,
                       int clientFd)
    : _pid(-1), _startTime(0),
      _timeoutSeconds(2), _finished(false), _inputClosed(false),
      _clientFd(clientFd), _scriptPath(scriptPath), _env(env) {
    
    // Inicializar pipes como inválidos (C++98 compatible)
    _pipeIn[0] = -1;
    _pipeIn[1] = -1;
    _pipeOut[0] = -1;
    _pipeOut[1] = -1;
}

CgiProcess::~CgiProcess() {
    killProcess();
    
    // Fechar pipes
    if (_pipeIn[1] != -1) {
        close(_pipeIn[1]);
        _pipeIn[1] = -1;
    }
    if (_pipeOut[0] != -1) {
        close(_pipeOut[0]);
        _pipeOut[0] = -1;
    }
}

bool CgiProcess::start() {
    // Criar pipes
    if (pipe(_pipeIn) == -1 || pipe(_pipeOut) == -1) {
        std::cerr << "CGI: Failed to create pipes" << std::endl;
        return false;
    }
    
    // Tornar pipes non-blocking
    fcntl(_pipeIn[1], F_SETFL, O_NONBLOCK);
    fcntl(_pipeOut[0], F_SETFL, O_NONBLOCK);
    
    // Fork
    _pid = fork();
    if (_pid == -1) {
        std::cerr << "CGI: Failed to fork" << std::endl;
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
        return true;
    }
}

std::string CgiProcess::getInterpreter(const std::string& scriptPath) const {
    // Extrair extensão
    size_t dotPos = scriptPath.find_last_of('.');
    if (dotPos == std::string::npos) {
        // Sem extensão - tentar executar diretamente
        return scriptPath;
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
        // Sem interpretador conhecido - tentar executar diretamente
        return scriptPath;
    }
}

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

void CgiProcess::setupParentProcess() {
    // Fechar extremidades não usadas
    close(_pipeIn[0]);   // Não lemos do pipe de entrada
    close(_pipeOut[1]);  // Não escrevemos no pipe de saída
}

bool CgiProcess::isFinished() const {
    if (_finished)
        return true;
    
    // Verificar se processo terminou
    if (_pid > 0) {
        int status;
        pid_t waitResult = waitpid(_pid, &status, WNOHANG);
        if (waitResult == _pid) {
            return true;
        }
    }
    
    return false;
}

bool CgiProcess::isTimedOut() const {
    if (_finished)
        return false;
    
    time_t currentTime = time(NULL);
    return (currentTime - _startTime) >= _timeoutSeconds;
}

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
            _inputClosed = true;
        }
        if (_pipeOut[0] != -1) {
            close(_pipeOut[0]);
        }
        
        _finished = true;
    }
}

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
        // Isso permite que scripts que leem do stdin (como POST) saibam que não há mais dados
        std::cout << "CGI: Closing input pipe after writing " << written << " bytes" << std::endl;
        close(_pipeIn[1]);
        _pipeIn[1] = -1;
        _inputClosed = true;
        std::cout << "CGI: Input pipe closed, _inputClosed = true" << std::endl;
        
        // Pequeno delay para garantir que o EOF seja propagado
        // Isso é necessário porque o processo filho pode estar bloqueado
        // esperando mais dados e precisa receber o EOF antes de continuar
        usleep(10000); // 10ms - tempo mínimo para o sistema propagar EOF
        
        return true;
    }
    
    // Escreveu parcialmente - atualizar remainingBody com o que sobrou
    _remainingBody = data.substr(written);
    return false;
}

bool CgiProcess::readFromStdout() {
    if (_finished || _pipeOut[0] == -1)
        return false;
    
    char buffer[8192];
    ssize_t bytesRead = read(_pipeOut[0], buffer, sizeof(buffer));
    
    if (bytesRead > 0) {
        _outputBuffer.append(buffer, bytesRead);
        std::cout << "CGI: Read " << bytesRead << " bytes from stdout (total: " << _outputBuffer.size() << ")" << std::endl;
        return true;
    } else if (bytesRead == 0) {
        // EOF - processo terminou de escrever
        std::cout << "CGI: EOF on stdout pipe" << std::endl;
        close(_pipeOut[0]);
        _pipeOut[0] = -1;
        
        // Quando recebemos EOF, o processo já terminou (ou está para terminar)
        // Marcar como finished para processar resposta
        _finished = true;
        
        // Tentar coletar o processo (não bloqueante)
        int status;
        pid_t waitResult = waitpid(_pid, &status, WNOHANG);
        if (waitResult == _pid) {
            std::cout << "CGI: Process " << _pid << " terminated with status " << status << std::endl;
        } else {
            std::cout << "CGI: Process " << _pid << " will be collected later" << std::endl;
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

std::string CgiProcess::getOutput() const {
    return _outputBuffer;
}

bool CgiProcess::isInputClosed() const {
    return _inputClosed;
}

int CgiProcess::getPipeInFd() const {
    return _pipeIn[1];
}

int CgiProcess::getPipeOutFd() const {
    return _pipeOut[0];
}

pid_t CgiProcess::getPid() const {
    return _pid;
}

int CgiProcess::getClientFd() const {
    return _clientFd;
}

std::string CgiProcess::getRemainingBody() const {
    return _remainingBody;
}

void CgiProcess::setRemainingBody(const std::string& body) {
    _remainingBody = body;
}

void CgiProcess::closeInputPipe() {
    if (!_inputClosed && _pipeIn[1] != -1) {
        close(_pipeIn[1]);
        _pipeIn[1] = -1;
        _inputClosed = true;
    }
}


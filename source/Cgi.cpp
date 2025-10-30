#include "../includes/WebservHeader.hpp"
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>
#include <sstream>

// Tamanho do buffer para leitura
#define CGI_BUFFER_SIZE 4096

/**
 * @brief Construtor da classe Cgi
 */
Cgi::Cgi(const std::string &scriptPath, HttpRequest &request, HttpResponse &response)
	: EpollHandler(-1, EPOLLOUT),  // Começa com EPOLLOUT para escrever
	  _pid(-1),
	  _startTime(0),
	  _scriptPath(scriptPath),
	  _body(request.getBody()),
	  _cgiOutput(""),
	  _totalBytesWritten(0),
	  _state(WRITING_TO_CGI),
	  _request(request),
	  _response(response) {
	
	_pipeFdIn[0] = -1;
	_pipeFdIn[1] = -1;
	_pipeFdOut[0] = -1;
	_pipeFdOut[1] = -1;
	
	std::cout << "[CGI] Creating CGI for script: " << _scriptPath << std::endl;
}

/**
 * @brief Destrutor - limpa recursos
 */
Cgi::~Cgi(void) {
	std::cout << "[CGI] Destroying CGI instance" << std::endl;
	
	// Liberar memória das variáveis de ambiente
	for (size_t i = 0; i < _envVars.size(); ++i) {
		if (_envVars[i] != NULL) {
			free(_envVars[i]);
		}
	}
	
	// Matar processo se ainda estiver rodando
	if (_pid > 0) {
		killProcess();
	}
	
	// Fechar pipes
	closePipes();
}

/**
 * @brief Cria e configura os pipes para comunicação com CGI
 */
void Cgi::setupPipes(void) {
	// Criar pipe para stdin (escrita)
	if (pipe(_pipeFdIn) == -1) {
		throw PipeException();
	}
	
	// Criar pipe para stdout (leitura)
	if (pipe(_pipeFdOut) == -1) {
		close(_pipeFdIn[0]);
		close(_pipeFdIn[1]);
		throw PipeException();
	}
	
	// Configurar pipe de escrita como não-bloqueante
	int flags = fcntl(_pipeFdIn[1], F_GETFL, 0);
	if (flags == -1 || fcntl(_pipeFdIn[1], F_SETFL, flags | O_NONBLOCK) == -1) {
		closePipes();
		throw PipeException();
	}
	
	// Configurar pipe de leitura como não-bloqueante
	flags = fcntl(_pipeFdOut[0], F_GETFL, 0);
	if (flags == -1 || fcntl(_pipeFdOut[0], F_SETFL, flags | O_NONBLOCK) == -1) {
		closePipes();
		throw PipeException();
	}
	
	std::cout << "[CGI] Pipes created: stdin=" << _pipeFdIn[1] 
	          << " stdout=" << _pipeFdOut[0] << std::endl;
}

/**
 * @brief Fecha todos os pipes
 */
void Cgi::closePipes(void) {
	if (_pipeFdIn[0] != -1) {
		close(_pipeFdIn[0]);
		_pipeFdIn[0] = -1;
	}
	if (_pipeFdIn[1] != -1) {
		close(_pipeFdIn[1]);
		_pipeFdIn[1] = -1;
	}
	if (_pipeFdOut[0] != -1) {
		close(_pipeFdOut[0]);
		_pipeFdOut[0] = -1;
	}
	if (_pipeFdOut[1] != -1) {
		close(_pipeFdOut[1]);
		_pipeFdOut[1] = -1;
	}
}

/**
 * @brief Constrói as variáveis de ambiente CGI
 */
void Cgi::buildEnvVars(void) {
	std::vector<std::string> envStrings;
	
	// Variáveis obrigatórias CGI/1.1
	envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envStrings.push_back("SERVER_SOFTWARE=WebServ/1.0");
	envStrings.push_back("SERVER_PROTOCOL=HTTP/1.1");
	envStrings.push_back("REQUEST_METHOD=" + _request.getMethod());
	envStrings.push_back("SCRIPT_FILENAME=" + _scriptPath);
	envStrings.push_back("REDIRECT_STATUS=200");  // Para PHP
	
	// Query string (parte depois do ?)
	std::string uri = _request.getUri();
	size_t qPos = uri.find('?');
	if (qPos != std::string::npos) {
		std::string queryString = uri.substr(qPos + 1);
		envStrings.push_back("QUERY_STRING=" + queryString);
		envStrings.push_back("PATH_INFO=" + uri.substr(0, qPos));
	} else {
		envStrings.push_back("QUERY_STRING=");
		envStrings.push_back("PATH_INFO=" + uri);
	}
	
	// Content-Type e Content-Length para POST
	std::string contentType = _request.getHeaderValue("Content-Type");
	if (!contentType.empty()) {
		envStrings.push_back("CONTENT_TYPE=" + contentType);
	}
	
	std::string contentLength = _request.getHeaderValue("Content-Length");
	if (!contentLength.empty()) {
		envStrings.push_back("CONTENT_LENGTH=" + contentLength);
	}
	
	// Cookies (para o bônus 2)
	std::string cookie = _request.getHeaderValue("Cookie");
	if (!cookie.empty()) {
		envStrings.push_back("HTTP_COOKIE=" + cookie);
	}
	
	// Converter para array de char* (formato esperado por execve)
	for (size_t i = 0; i < envStrings.size(); ++i) {
		_envVars.push_back(strdup(envStrings[i].c_str()));
		std::cout << "[CGI] ENV: " << envStrings[i] << std::endl;
	}
	_envVars.push_back(NULL);  // Terminar com NULL
}

/**
 * @brief Código executado no processo filho (script CGI)
 */
void Cgi::executeChild(void) {
	// Fechar extremidades não usadas dos pipes
	close(_pipeFdIn[1]);   // Não escreve no stdin
	close(_pipeFdOut[0]);  // Não lê do stdout
	
	// Redirecionar stdin e stdout
	if (dup2(_pipeFdIn[0], STDIN_FILENO) == -1) {
		std::cerr << "[CGI] Error: dup2 stdin failed" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	if (dup2(_pipeFdOut[1], STDOUT_FILENO) == -1) {
		std::cerr << "[CGI] Error: dup2 stdout failed" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	// Fechar pipes originais (já duplicados)
	close(_pipeFdIn[0]);
	close(_pipeFdOut[1]);
	
	// Preparar argumentos para execve
	char *argv[2];
	argv[0] = const_cast<char*>(_scriptPath.c_str());
	argv[1] = NULL;
	
	// Executar script
	std::cout << "[CGI] Child process executing: " << _scriptPath << std::endl;
	execve(_scriptPath.c_str(), argv, &_envVars[0]);
	
	// Se chegou aqui, execve falhou
	std::cerr << "[CGI] Error: execve failed for " << _scriptPath 
	          << ": " << strerror(errno) << std::endl;
	exit(EXIT_FAILURE);
}

/**
 * @brief Configura o processo pai após o fork
 */
void Cgi::setupParent(void) {
	// Fechar extremidades não usadas
	close(_pipeFdIn[0]);   // Não lê do stdin do CGI
	close(_pipeFdOut[1]);  // Não escreve no stdout do CGI
	_pipeFdIn[0] = -1;
	_pipeFdOut[1] = -1;
	
	// Se não há body para enviar, fechar escrita e mudar para leitura
	if (_body.empty()) {
		close(_pipeFdIn[1]);
		_pipeFdIn[1] = -1;
		this->setSocketFd(_pipeFdOut[0]);
		_state = READING_FROM_CGI;
		std::cout << "[CGI] No body to send, switching to read mode" << std::endl;
	} else {
		// Há body para enviar (POST)
		this->setSocketFd(_pipeFdIn[1]);
		_state = WRITING_TO_CGI;
		std::cout << "[CGI] Body to send (" << _body.size() << " bytes)" << std::endl;
	}
}

/**
 * @brief Executa o script CGI (fork + execve)
 */
void Cgi::execute(void) {
	std::cout << "[CGI] Starting execution of " << _scriptPath << std::endl;
	
	// Criar pipes
	setupPipes();
	
	// Construir variáveis de ambiente
	buildEnvVars();
	
	// Marcar tempo de início (para timeout)
	_startTime = time(NULL);
	
	// Fork
	_pid = fork();
	
	if (_pid == -1) {
		closePipes();
		throw ForkException();
	}
	
	if (_pid == 0) {
		// Processo filho
		executeChild();
		// Nunca retorna daqui
	}
	
	// Processo pai
	std::cout << "[CGI] Forked CGI process, PID: " << _pid << std::endl;
	setupParent();
}

/**
 * @brief Trata evento EPOLLOUT (pronto para escrever)
 */
void Cgi::handleEpollOut(void) {
	if (_state != WRITING_TO_CGI) {
		return;
	}
	
	// Escrever body no stdin do CGI
	size_t remaining = _body.size() - _totalBytesWritten;
	if (remaining == 0) {
		// Terminamos de escrever, fechar pipe de escrita
		std::cout << "[CGI] Finished writing body" << std::endl;
		close(_pipeFdIn[1]);
		_pipeFdIn[1] = -1;
		
		// Mudar para modo de leitura
		this->setSocketFd(_pipeFdOut[0]);
		_state = READING_FROM_CGI;
		return;
	}
	
	ssize_t bytesWritten = write(_pipeFdIn[1], 
	                              _body.c_str() + _totalBytesWritten, 
	                              remaining);
	
	if (bytesWritten == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			// Pipe cheio, tentar novamente depois
			return;
		}
		std::cerr << "[CGI] Error writing to pipe: " << strerror(errno) << std::endl;
		_state = DONE;
		_response.setErrorPage(500);
		return;
	}
	
	_totalBytesWritten += bytesWritten;
	std::cout << "[CGI] Wrote " << bytesWritten << " bytes (" 
	          << _totalBytesWritten << "/" << _body.size() << ")" << std::endl;
}

/**
 * @brief Trata evento EPOLLIN (pronto para ler)
 */
void Cgi::handleEpollIn(void) {
	if (_state != READING_FROM_CGI) {
		return;
	}
	
	char buffer[CGI_BUFFER_SIZE];
	ssize_t bytesRead = read(_pipeFdOut[0], buffer, CGI_BUFFER_SIZE);
	
	if (bytesRead == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			// Sem dados disponíveis, tentar depois
			return;
		}
		std::cerr << "[CGI] Error reading from pipe: " << strerror(errno) << std::endl;
		_state = DONE;
		_response.setErrorPage(500);
		return;
	}
	
	if (bytesRead == 0) {
		// EOF - processo terminou
		std::cout << "[CGI] EOF reached, total output: " << _cgiOutput.size() << " bytes" << std::endl;
		
		// Esperar pelo processo filho
		int status;
		waitpid(_pid, &status, 0);
		_pid = -1;
		
		// Verificar se terminou com sucesso
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
			std::cerr << "[CGI] Process exited with error" << std::endl;
			_response.setErrorPage(500);
		} else {
			// Processar output do CGI
			parseOutput();
		}
		
		_state = DONE;
		return;
	}
	
	// Adicionar dados lidos ao output
	_cgiOutput.append(buffer, bytesRead);
	std::cout << "[CGI] Read " << bytesRead << " bytes" << std::endl;
}

/**
 * @brief Parseia o output do CGI (headers + body)
 */
void Cgi::parseOutput(void) {
	std::cout << "[CGI] Parsing CGI output (" << _cgiOutput.size() << " bytes)" << std::endl;
	
	// Procurar por linha vazia que separa headers de body
	size_t headerEnd = _cgiOutput.find("\r\n\r\n");
	if (headerEnd == std::string::npos) {
		// Tentar com \n\n (alguns scripts usam apenas \n)
		headerEnd = _cgiOutput.find("\n\n");
		if (headerEnd == std::string::npos) {
			std::cerr << "[CGI] Invalid output: no header/body separator" << std::endl;
			_response.setErrorPage(500);
			return;
		}
		headerEnd += 2;  // \n\n
	} else {
		headerEnd += 4;  // \r\n\r\n
	}
	
	// Separar headers e body
	std::string headersStr = _cgiOutput.substr(0, headerEnd);
	std::string body = _cgiOutput.substr(headerEnd);
	
	// Parsear headers
	std::istringstream headerStream(headersStr);
	std::string line;
	std::string contentType = "text/html";  // Default
	
	while (std::getline(headerStream, line)) {
		// Remover \r se presente
		if (!line.empty() && line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1);
		}
		
		if (line.empty()) continue;
		
		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos) continue;
		
		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);
		
		// Trim espaços
		while (!value.empty() && (value[0] == ' ' || value[0] == '\t')) {
			value.erase(0, 1);
		}
		
		// Converter key para lowercase
		for (size_t i = 0; i < key.size(); ++i) {
			key[i] = std::tolower(key[i]);
		}
		
		std::cout << "[CGI] Header: " << key << " = " << value << std::endl;
		
		// Processar headers importantes
		if (key == "content-type") {
			contentType = value;
		} else if (key == "status") {
			// Status header (ex: "Status: 404 Not Found")
			// Por enquanto, ignoramos e usamos 200
		}
		// Outros headers (Set-Cookie, etc) podem ser adicionados aqui
	}
	
	// Configurar resposta
	_response.setBody(body, contentType);
	_response.setStatus(200, "OK");
	
	std::cout << "[CGI] Response configured: " << body.size() << " bytes, " 
	          << contentType << std::endl;
}

/**
 * @brief Verifica se o processo excedeu o timeout
 */
bool Cgi::isTimedOut(int maxSeconds) {
	if (_startTime == 0) return false;
	time_t now = time(NULL);
	return (now - _startTime) > maxSeconds;
}

/**
 * @brief Mata o processo CGI
 */
void Cgi::killProcess(void) {
	if (_pid > 0) {
		std::cout << "[CGI] Killing process " << _pid << std::endl;
		kill(_pid, SIGKILL);
		waitpid(_pid, NULL, 0);
		_pid = -1;
	}
}

/**
 * @brief Verifica se o CGI terminou
 */
bool Cgi::isDone(void) const {
	return _state == DONE;
}

/**
 * @brief Getters
 */
pid_t Cgi::getPid(void) const {
	return _pid;
}

int Cgi::getReadFd(void) const {
	return _pipeFdOut[0];
}

int Cgi::getWriteFd(void) const {
	return _pipeFdIn[1];
}

/**
 * @brief Exceções
 */
const char *Cgi::PipeException::what() const throw() {
	return "Error: Failed to create or configure CGI pipes.";
}

const char *Cgi::ForkException::what() const throw() {
	return "Error: Failed to fork CGI process.";
}

const char *Cgi::ExecException::what() const throw() {
	return "Error: Failed to execute CGI script.";
}

const char *Cgi::TimeoutException::what() const throw() {
	return "Error: CGI script exceeded timeout limit.";
}


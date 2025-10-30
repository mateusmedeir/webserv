#pragma once

#include "WebservHeader.hpp"

class HttpRequest;
class HttpResponse;

/**
 * @brief Classe para executar scripts CGI (.py, .php, etc)
 * 
 * Herda de EpollHandler para integração com o sistema de eventos.
 * Gerencia fork/execve de processos CGI com pipes não-bloqueantes.
 */
class Cgi : public EpollHandler {
	private:
		// Pipes para comunicação com o processo CGI
		int _pipeFdIn[2];      // Pipe para stdin do script (write)
		int _pipeFdOut[2];     // Pipe para stdout do script (read)
		
		// Processo CGI
		pid_t _pid;            // PID do processo filho
		time_t _startTime;     // Timestamp do início (para timeout)
		
		// Dados da requisição/resposta
		std::string _scriptPath;
		std::string _body;           // Body a ser enviado (POST)
		std::string _cgiOutput;      // Output do CGI
		size_t _totalBytesWritten;   // Bytes escritos no stdin
		
		// Variáveis de ambiente CGI
		std::vector<char*> _envVars;
		
		// Estados do CGI
		enum CgiState {
			WRITING_TO_CGI,   // Escrevendo body no stdin
			READING_FROM_CGI, // Lendo output do stdout
			DONE              // Processo terminado
		};
		CgiState _state;
		
		// Referências
		HttpRequest &_request;
		HttpResponse &_response;
		
		// Métodos privados
		void setupPipes(void);
		void closePipes(void);
		void executeChild(void);
		void setupParent(void);
		void buildEnvVars(void);
		void parseOutput(void);
		
	public:
		Cgi(const std::string &scriptPath, HttpRequest &request, HttpResponse &response);
		~Cgi(void);
		
		// Override de EpollHandler
		virtual void handleEpollIn(void);
		virtual void handleEpollOut(void);
		
		// Métodos públicos
		void execute(void);
		bool isTimedOut(int maxSeconds);
		void killProcess(void);
		bool isDone(void) const;
		
		// Getters
		pid_t getPid(void) const;
		int getReadFd(void) const;
		int getWriteFd(void) const;
		
		// Exceções
		class PipeException : public std::exception {
			public:
				virtual const char *what() const throw();
		};
		
		class ForkException : public std::exception {
			public:
				virtual const char *what() const throw();
		};
		
		class ExecException : public std::exception {
			public:
				virtual const char *what() const throw();
		};
		
		class TimeoutException : public std::exception {
			public:
				virtual const char *what() const throw();
		};
};


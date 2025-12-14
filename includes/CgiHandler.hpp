#pragma once

#include "WebservHeader.hpp"

class CgiHandler: public EpollHandler {

    private:
				int					_fdIn[2];
				int					_fdOut[2];
				std::string _scriptPath;
				std::vector<std::string> _env;
				std::string _cgiOutput;
				bool _isFinished;
				pid_t _childPid; // Adicionado para armazenar o PID do processo filho

				const HttpRequest &_request;
				const ServerBlock	&_serverBlock;
				const LocationBlock	&_location;

    public:
        CgiHandler(
					const HttpRequest &request,
					const ServerBlock &serverBlock,
					const LocationBlock &location
				);
        ~CgiHandler();

				std::vector<std::string> buildEnvironment();
				bool start();

				virtual void handleEpollIn();
				virtual void handleEpollOut();

				const std::string& getCgiOutput() const;
				bool isFinished() const;
				pid_t getChildPid() const; // Getter para o PID do processo filho

				static bool isCgiScript(const std::string& uri, const LocationBlock& location);
				static std::string extractCgiScriptName(const std::string& uri);
				static std::string extractQueryString(const std::string& uri);
				std::string extractCgiScriptPath(const std::string& uri);

				std::string normalizeHeaderName(const std::string& header);
				std::string intToString(int n);

				std::string getInterpretterPath(const std::string& scriptPath);
};
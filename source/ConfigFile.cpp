#include "../includes/WebservHeader.hpp"

ConfigFile::ConfigFile(void) {}

ConfigFile::~ConfigFile(void) {}

// ConfigFile(char **av, int ac);
ConfigFile::ConfigFile(char **av, int ac) {
	if (ac == 2)
        this->parser(av[1]);
    else //| Caso não passem nenhum argumento, vamos usar nosso arquivo padrão
        this->parser("configs/test_simple.conf");
}

void ConfigFile::readFile(const std::string &filename, std::string &content)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
		throw std::runtime_error("Could not open file: " + filename);

	content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void ConfigFile::trim(std::string &content)
{
	size_t start = content.find_first_not_of(" \t\n\r\f\v");
	if (start == std::string::npos)
	{
		content.clear();
		return;
	}
	
	size_t end = content.find_last_not_of(" \t\n\r\f\v");
	
	content = content.substr(start, end - start + 1);
}

void ConfigFile::removeComments(std::string &content)
{
	std::string result;
	std::istringstream iss(content);
	std::string line;
	
	while (std::getline(iss, line))
	{
		size_t commentPos = line.find('#');
		
		if (commentPos != std::string::npos)
			line = line.substr(0, commentPos);
		
		result += line + "\n";
	}
	
	if (!result.empty() && result[result.length() - 1] == '\n')
		result.erase(result.length() - 1);
	
	content = result;
}

void ConfigFile::cleanFile(const std::string &filename, std::string &content)
{
	readFile(filename, content); //| Arquivo completo
	removeComments(content);     //| Remover comentários (linhas com '#')
	trim(content);               //| Remover os whitespaces do começo e do final do content.
}

std::vector<std::string> ConfigFile::tokenizeContent(const std::string &content)
{
	std::string currentToken;
	bool inQuotes = false;
	char quoteChar = '\0';
	int braceCount = 0;
	
	for (size_t i = 0; i < content.length(); ++i)
	{
		char c = content[i];

		if ((c == '"' || c == '\'') && !inQuotes) //| Verifica se o caractere é uma aspas
		{
			inQuotes = true;
			quoteChar = c;
			if (!currentToken.empty()) //| Adiciona o token atual ao vetor de tokens
			{
				this->_tokens.push_back(currentToken);
				currentToken.clear();
			}
			currentToken += c;
		}
		else if (c == quoteChar && inQuotes) //| Verifica se o caractere é a aspas que fecha
		{
			inQuotes = false;
			currentToken += c;
			this->_tokens.push_back(currentToken);
			currentToken.clear();
			quoteChar = '\0';
		}
		else if (inQuotes) //| Verifica se o caractere está dentro de aspas, se estiver, só passa para o próximo caractere
			currentToken += c;
		else if (c == '{' || c == '}' || c == ';') //| Verifica se o caractere é uma chave ou ponto e vírgula
		{
			if (!inQuotes) //| Conta a quantidade de chaves abertas e fechadas
			{
				if (c == '{')
					braceCount++;
				else if (c == '}')
					braceCount--;
			}
			if (!currentToken.empty()) //| Adiciona o token atual ao vetor de tokens
			{
				this->_tokens.push_back(currentToken);
				currentToken.clear();
			}
			this->_tokens.push_back(std::string(1, c));
		}
		else if (std::isspace(c)) //| Verifica se o caractere é um espaço
		{
			if (!currentToken.empty()) //| Adiciona o token atual ao vetor de tokens
			{
				this->_tokens.push_back(currentToken);
				currentToken.clear();
			}
		}
		else //| Se não for um espaço, adiciona o caractere ao token atual
			currentToken += c;
	}
	
	if (!currentToken.empty()) //| Adiciona o token atual ao vetor de tokens
		this->_tokens.push_back(currentToken);
	
	if (braceCount != 0) //| Verifica se as chaves estão balanceadas
		throw std::runtime_error("Configuração inválida: chaves não balanceadas");
	
	return this->_tokens;
}

void ConfigFile::parser(const std::string &filename)
{
	std::string content;
	cleanFile(filename, content);
	tokenizeContent(content);

	if (this->_tokens.size() == 0)
		throw std::runtime_error("Configuração inválida: não foi encontrado nenhum servidor");

	while (this->_tokens.size() > 0) //| While para pegar todos os servers (se tiver mais de um server)
	{
		if (this->_tokens[0] == "server" && this->_tokens[1] == "{")
			this->_serverBlocks.push_back(ServerBlock(*this));
		else
			throw std::runtime_error("Configuração inválida: servidor não encontrado");
	}
}

void ConfigFile::removeTokens(size_t amount)
{
	if (!this->_tokens.empty())
		this->_tokens.erase(this->_tokens.begin(), this->_tokens.begin() + amount);
}

void ConfigFile::verifyToken(TypeValidation type, const std::string &message)
{
	bool shouldThrow = false;

	switch (type)
	{
		case EMPTY:
			shouldThrow = this->_tokens.empty();
			break;
		case SEMICOLON:
			shouldThrow = this->_tokens.empty() || this->_tokens[0] == ";";
			break;
		case DIFF_SEMICOLON:
			shouldThrow = this->_tokens.empty() || this->_tokens[0] != ";";
			break;
		case END_OF_FILE:
			shouldThrow = this->_tokens[0] == this->_tokens.back();
			break;
		default:
			throw std::runtime_error("Configuração inválida: tipo de validação desconhecido");
	}

	if (shouldThrow)
		throw std::runtime_error(message);
}

std::vector<std::string> ConfigFile::getTokens(void)
{
	return this->_tokens;
}

const std::vector<ServerBlock> &ConfigFile::getServerBlocks(void) const
{
	return this->_serverBlocks;
}

#include "ParserConfigFile.hpp"
#include <sstream>

void	ParserConfigFile::readFile(const std::string &filename, std::string &content)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
		throw std::runtime_error("Could not open file: " + filename);

	content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void	ParserConfigFile::trim(std::string &content)
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

void	ParserConfigFile::removeComments(std::string &content)
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

void ParserConfigFile::cleanFile(const std::string &filename, std::string &content)
{
	readFile(filename, content); //| Arquivo completo
	removeComments(content);     //| Remover comentários (linhas com '#')
	trim(content);               //| Remover os whitespaces do começo e do final do content.
}

/*
void	ParserConfigFile::readFile(const std::string &filename, std::string &content)
{
	std::ifstream file(filename.c_str(), std::ios::binary);
	if (!file.is_open())
		throw std::runtime_error("Could not open file: " + filename);
	
	file.seekg(0, std::ios::end);
	std::size_t size = file.tellg();

	file.seekg(0, std::ios::beg);

	content.assign(static_cast<size_t>(size), '\0');
	file.read(&content[0], size);
	if (!file)
		throw std::runtime_error("Error reading file: " + filename);

	file.close();
}

void	ParserConfigFile::readFile(const std::string &filename, std::string &content)
{
	int fd = open(filename.c_str(), O_RDONLY);
	if (fd < 0)
		throw std::runtime_error("Could not open file: " + filename);

	struct stat st;
	if (fstat(fd, &st) < 0)
	{
		close(fd);
		throw std::runtime_error("Could not get file size: " + filename);
	}
	size_t size = st.st_size;

	content.resize(size);

	ssize_t bytesRead = 0;
	size_t totalRead = 0;
	while (totalRead < size)
	{
		bytesRead = read(fd, &content[totalRead], size - totalRead);
		if (bytesRead < 0)
		{
			close(fd);
			throw std::runtime_error("Error reading file: " + filename);
		}
		if (bytesRead == 0) //| EOF
			break;
		totalRead += bytesRead;
	}

	close(fd);

	if (totalRead != size)
	
	content.resize(totalRead);
}
*/
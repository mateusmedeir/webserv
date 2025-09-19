#include "ParserConfigFile.hpp"
#include <iostream>

int	verify_args(int ac, char **av)
{
	if (ac != 2 || std::string(av[1]).empty())
	{
		std::cerr << "Usage: " << av[0] << " <config_file>" << std::endl;
		return (0);
	}
	return (1);
}

int	main(int ac, char **av)
{
	if (!verify_args(ac, av))
		return (1);

	std::string	content;
	ParserConfigFile::readFile(av[1], content);
	
	// Arquivo completo
	std::cout << "----- File Content Start -----" << std::endl;
	std::cout << content << std::endl;

	// Remover os whitespaces do começo e do final do content.
	ParserConfigFile::trim(content);
	std::cout << "----- Trimmed Content Start -----" << std::endl;
	std::cout << content << std::endl;

}

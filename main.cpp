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

	std::string content;
	ParserConfigFile::cleanFile(av[1], content);
	std::cout << content << std::endl;

	return (0);
}

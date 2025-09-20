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

void	print_tokens(std::vector<std::string> tokens)
{
	std::cout << "|=== TOKENS ===|" << std::endl;
	for (size_t i = 0; i < tokens.size(); ++i)
		std::cout << "[" << i << "] = '" << tokens[i] << "'" << std::endl;
	std::cout << "Total tokens: " << tokens.size() << std::endl;
}

int	main(int ac, char **av)
{
	if (!verify_args(ac, av))
		return (1);

	std::vector<std::string> tokens;
	ParserConfigFile::parser(av[1], tokens);
	
	print_tokens(tokens);

	return (0);
}

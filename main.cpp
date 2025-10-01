#include "ParserConfigFile.hpp"
#include <iostream>

#include "ServerBlock.hpp"

int	verify_args(int ac, char **av)
{
	if (ac > 2)
	{
		std::cerr << "Usage: " << av[0] << " <config_file>\n\tOR\nUsage: " << av[0] << std::endl;
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

	try
	{
		std::vector<std::string> tokens;
		if (ac == 2)
			ParserConfigFile::parser(av[1], tokens);
		else //| Caso não passem nenhum argumento, vamos usar nosso arquivo padrão
			ParserConfigFile::parser("Configures/test_simple.conf", tokens);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error! Webserv: " << e.what() << std::endl;
		return (1);
	}

	return (0);
}

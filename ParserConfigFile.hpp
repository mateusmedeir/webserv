#pragma once

#include <string>     // std::string
#include <fstream>    // std::ifstream

class ParserConfigFile
{
	public:
		static void readFile(const std::string &filename, std::string &content);
};

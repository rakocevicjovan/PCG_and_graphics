#pragma once
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <string>

namespace FileUtils
{
	static std::string loadFileContents(const std::string& path)
	{
		std::ifstream t(path);
		std::stringstream buffer;
		buffer << t.rdbuf();

		return buffer.str();
	}
}
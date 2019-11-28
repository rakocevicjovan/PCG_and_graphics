#pragma once
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <string>

namespace FileUtils
{
	inline static std::string loadFileContents(const std::string& path)
	{
		std::ifstream t(path);
		std::stringstream buffer;
		buffer << t.rdbuf();

		return buffer.str();
	}

	inline static bool fileExists(const std::string& name)
	{
		std::ifstream f(name.c_str());
		return f.good();
	}
}
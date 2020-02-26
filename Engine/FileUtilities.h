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

	static void writeToFile(const std::string& fileName, void* data, size_t size)
	{
		std::ofstream fout;
		fout.open(fileName, std::ios::binary | std::ios::out);
		fout.write(reinterpret_cast<char*>(data), size);
		fout.close();
	}
}
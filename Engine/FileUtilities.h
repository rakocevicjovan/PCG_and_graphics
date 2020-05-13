#pragma once
#include <fstream>
#include <sstream>
#include <iostream>

#include <string>

#include <filesystem>
#include <dirent.h>	// Using 17 now so not necessary?



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



	static void printDirContents(const std::string& path)
	{
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			std::cout << entry.path().string() << std::endl;
		}
	}



	static bool getDirContentsAsStrings(const std::string& path, std::vector<std::string>& contents)
	{
		if (!std::filesystem::is_directory(path))
			return false;

		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			contents.push_back(entry.path().filename().string());
		}

		return true;
	}



	static std::filesystem::directory_iterator getDirIterator(const std::string& path)
	{
		return std::filesystem::directory_iterator(path);
	}
}
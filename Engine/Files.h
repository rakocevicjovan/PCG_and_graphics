#pragma once
#include <iostream>
#include <fstream>
#include <string>

class Files
{
public:

	//char should be 1 byte in most implementations but I don't like this
	static void writeToFile(const std::string& fileName, void* data, size_t size)
	{
		std::ofstream fout;

		fout.open(fileName, std::ios::binary | std::ios::out);

		fout.write(reinterpret_cast<char*>(data), size);

		fout.close();
	}


};

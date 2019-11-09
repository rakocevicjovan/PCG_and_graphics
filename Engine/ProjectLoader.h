#pragma once
#include <string>

class ProjectLoader
{
private:
	std::string _projConfPath;

public:
	ProjectLoader(const std::string& projConfPath);

	std::string getProjConfpath();
};
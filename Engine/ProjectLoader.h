#pragma once
#include <string>

//not really required but meh, not a big overhead to store this
struct ProjectConfiguration
{
	unsigned int id;
	std::string _projectName;
	std::string _projectPath;
	std::string _description;
	std::string _createdAt;
	std::string _updatedAt;
};



class ProjectLoader
{
private:
	ProjectConfiguration _pc;
	std::string _projConfPath;

public:

	ProjectLoader();

	bool loadConfig(const std::string&& projConfPath);
	const std::string& getProjbasepath() { return _pc._projectPath; }
};
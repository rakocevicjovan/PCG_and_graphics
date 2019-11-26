#pragma once
#include <string>
#include <vector>
#include "rapidjson/document.h"

//not really required but meh, not a big overhead to store this
struct ProjectConfiguration
{
	unsigned int id;
	int numLevels;
	std::string _projectName;
	std::string _projectPath;
	std::string _description;
	std::string _createdAt;
	std::string _updatedAt;
	std::vector<std::string> _levelList;
};



class ProjectLoader
{
private:
	ProjectConfiguration _pc;
	std::string _projConfPath;

	bool loadProjectConfiguration(const rapidjson::Document& projConfDoc);
	bool loadLevelList(const rapidjson::Document& projConfDoc);

public:

	ProjectLoader();

	bool loadProjFromConfig(const std::string&& projConfPath);

	const std::string& getProjDir();

	const std::vector<std::string>& getLevelList();
};

//std::string _assetDirPath;
//std::string _levelsDirPath;
//const std::string& ProjectLoader::getAssetDirPath() { return _assetDirPath; }
//const std::string& ProjectLoader::getLevelDirPath() { return _levelsDirPath; }
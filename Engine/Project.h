#pragma once
#include <string>
#include <vector>
#include "rapidjson/document.h"


struct ProjectDefinition
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



class Project
{
	std::string _projConfPath;
	ProjectDefinition _projDef;

public:

	Project() : _projDef({}) {}

	const std::string& getProjDir() const;
	const ProjectDefinition& getProjDef() const;
	const std::vector<std::string>& getLevelList() const;

	bool loadProjFromConfig(const std::string& projConfPath);
	bool loadProjectConfiguration(const rapidjson::Document& projConfDoc);
	bool loadLevelList(const rapidjson::Document& projConfDoc);
};
#pragma once
#include <string>
#include <vector>
#include "rapidjson/document.h"
#include "LevelReader.h"


struct ProjectDefinition
{
	unsigned int _ID;
	std::string _projectName;
	std::string _projectPath;
	std::string _description;
	std::string _createdAt;
	std::string _updatedAt;

	int _numLevels;
	std::vector<std::string> _levelList;
};



class Project
{
	std::string _configPath;
	ProjectDefinition _projDef;

	LevelReader _levelReader;

public:

	Project() : _projDef({}) {}

	const std::string& getProjDir() const;
	const ProjectDefinition& getProjDef() const;
	const std::vector<std::string>& getLevelList() const;
	LevelReader& getLevelReader() { return _levelReader; }

	bool loadFromConfig(const std::string& projConfPath);
	bool loadProjectConfiguration(const rapidjson::Document& projConfDoc);
	bool loadLevelList(const rapidjson::Document& projConfDoc);
};
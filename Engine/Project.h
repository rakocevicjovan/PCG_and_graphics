#pragma once

#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#include "rapidjson/document.h"
#undef _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

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
	std::string _configPath{};
	ProjectDefinition _projDef{};

	LevelReader _levelReader{};

public:

	std::string _ledgerPath;

	bool loadFromConfig(const std::string& projConfPath);
	bool loadProjectConfiguration(const rapidjson::Document& projConfDoc);
	bool loadLevelList(const rapidjson::Document& projConfDoc);

	inline const std::string&				getProjDir() const { return _projDef._projectPath; };
	inline const ProjectDefinition&			getProjDef() const { return _projDef; };
	inline const std::vector<std::string>&	getLevelList() const { return _projDef._levelList; };
	inline LevelReader&						getLevelReader() { return _levelReader; }
};
#pragma once
#include <vector>
#include <map>
#include "rapidjson/document.h"
#include "ResourceDef.h"





struct LevelDef
{
	int id;
	std::string name;
	std::string description;
	std::string projName;
	std::string jsonPath;
	std::string createdAt;
	std::string updatedAt;
};



class LevelReader
{
private:
	static const std::map<std::string, ResType> resTypeMap;
	ResType getResTypeFromString(const std::string& str);

	std::string _projectPath;
	LevelDef _ld;
	std::vector<ResourceDef> _resourceDefs;

	bool loadLevelDef(const rapidjson::Document& sceneDef);
	bool loadResourceDefs(const rapidjson::Document& sceneDef);

public:
	LevelReader();

	void setProjectPath(const std::string& projectPath);

	bool loadLevel(const std::string& levelPath);
	
	const std::vector<ResourceDef>& getLevelResourceDefs();
	void clearLevelResourceDefs();
};
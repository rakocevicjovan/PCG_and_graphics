#pragma once
#include <string>
#include <vector>
#include "rapidjson/document.h"


enum class ResType
{
	MESH,
	TEXTURE,
	MATERIAL,
	ANIMATION,
	SKELETON,
	SOUND
};



struct ResourceDef
{
	int id;
	std::string path;
	std::string name;
	ResType type;
};



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
	std::string _projectPath;
	std::vector<ResourceDef> _resourceDefs;
	LevelDef _ld;

	bool loadLevelDef(const rapidjson::Document& sceneDef);
	bool loadResourceDefs(const rapidjson::Document& sceneDef);

public:
	LevelReader();

	void setProjectData(const std::string& projectPath);

	bool loadLevel(const std::string& levelPath);
	
	const std::vector<ResourceDef>& getLevelResourceDefs();
	void clearLevelResourceDefs();
};
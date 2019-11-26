#pragma once
#include <string>
#include <vector>
#include "rapidjson/document.h"	//reeee... could try to forward spec it but not sure how, lots of templates


struct AssetDef
{
	int id;
	std::string path;
	std::string name;
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



class AssetLoader
{
private:
	std::string _projectPath;
	std::vector<AssetDef> _assetDefs;
	LevelDef _ld;

	bool loadLevelDef(const rapidjson::Document& sceneDef);
	bool loadAssetDefs(const rapidjson::Document& sceneDef);
	bool AssetLoader::loadAsset(const AssetDef& ad);

public:
	AssetLoader();

	bool loadLevel(const std::string& levelPath);
	void setProjectData(const std::string& projectPath);	//, const std::vector<std::string>& levelPaths
};
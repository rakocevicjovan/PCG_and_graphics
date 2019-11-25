#pragma once
#include <string>
#include <vector>



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
	static const std::string ASSET_SUFFIX;
	static const std::string LEVEL_SUFFIX;

private:
	std::string _assetDirPath;
	std::string _levelsDirPath;
	std::vector<std::string> _levelList;

	LevelDef _ld;
	std::vector<AssetDef> _assetDefs;

public:
	AssetLoader();

	void bindToProjectFolder(const std::string& basePath);
	bool loadLevelList();
	bool loadScene(const std::string& scenePath);

	std::string getAssetDirPath();
	std::string getLevelDirPath();
};
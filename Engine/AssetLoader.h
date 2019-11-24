#pragma once
#include <string>
#include <vector>

class AssetLoader
{
	static const std::string ASSET_SUFFIX;
	static const std::string LEVEL_SUFFIX;

private:
	std::string _assetDirPath;
	std::string _levelsDirPath;
	std::vector<std::string> _levelList;

public:
	AssetLoader();

	void bindToProjectFolder(const std::string& basePath);
	bool loadLevelList();
	void loadScene(const std::string& scenePath);

	std::string getAssetDirPath();
	std::string getLevelDirPath();
};
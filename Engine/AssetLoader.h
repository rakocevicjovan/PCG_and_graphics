#pragma once
#include <string>


class AssetLoader
{
private:
	std::string _modelDirPath;
	std::string _materialDirPath;

public:
	AssetLoader();

	void loadScene(const std::string& scenePath);

	std::string getModelDirPath();
	std::string getMaterialDirPath();
};
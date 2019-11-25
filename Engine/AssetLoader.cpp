#include "AssetLoader.h"

#include "rapidjson/document.h"

#include <dirent.h>
#include <fstream>
#include <sstream>


const std::string AssetLoader::ASSET_SUFFIX = "\\Assets";
const std::string AssetLoader::LEVEL_SUFFIX = "\\Levels";

AssetLoader::AssetLoader()
{

}



void AssetLoader::bindToProjectFolder(const std::string& basePath)
{
	_assetDirPath = basePath + ASSET_SUFFIX;
	_levelsDirPath = basePath + LEVEL_SUFFIX;
}



bool AssetLoader::loadLevelList()
{
	DIR *dir;
	struct dirent *entry;
	if ((dir = opendir(_levelsDirPath.c_str())) != NULL)
	{
		/* print all the files and directories within directory */
		while ((entry = readdir(dir)) != NULL)
		{
			std::string fname = entry->d_name;
			if (fname != "." && fname != "..")
			{
				printf("%s\n", fname);
				_levelList.push_back(fname);
			}
		}
		closedir(dir);
		return true;
	}
	else
	{
		/* could not open directory */
		perror("Could not open directory");
		return false;
	}
}



void AssetLoader::loadScene(const std::string& scenePath)
{
	rapidjson::Document sceneDef;

	std::ifstream t(scenePath);
	std::stringstream buffer;
	buffer << t.rdbuf();

	sceneDef.Parse(buffer.str().c_str());

	if (sceneDef.IsObject())
	{

	}
}



std::string AssetLoader::getAssetDirPath()
{
	return _assetDirPath;
}



std::string AssetLoader::getLevelDirPath()
{
	return _levelsDirPath;
}
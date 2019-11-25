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



bool AssetLoader::loadScene(const std::string& scenePath)
{
	rapidjson::Document sceneDef;

	std::ifstream t(scenePath);
	std::stringstream buffer;
	buffer << t.rdbuf();

	sceneDef.Parse(buffer.str().c_str());

	if (!sceneDef.IsObject())
		return false;
	
	//THANK YOU WINDOWS
	#pragma push_macro("GetObject")
	#undef GetObject

	auto levelDef	= sceneDef.FindMember("level")->value.GetObject();

	_ld.id			= levelDef.FindMember("id")->value.GetInt();
	_ld.name		= levelDef.FindMember("name")->value.GetString();
	_ld.description = levelDef.FindMember("description")->value.GetString();
	_ld.projName	= levelDef.FindMember("project")->value.GetString();
	_ld.jsonPath	= levelDef.FindMember("jsonDefPath")->value.GetString();
	_ld.createdAt	= levelDef.FindMember("createdAt")->value.GetString();
	_ld.updatedAt	= levelDef.FindMember("updatedAt")->value.GetString();

	#pragma pop_macro("MACRONAME")

	auto assDefArr = sceneDef.FindMember("asset_list")->value.GetArray();

	AssetDef ad;
	for (rapidjson::Value::ConstValueIterator itr = assDefArr.Begin(); itr != assDefArr.End(); ++itr)
	{
		ad.id	= itr->FindMember("id")->value.GetInt();
		ad.path = itr->FindMember("path")->value.GetString();
		ad.name = itr->FindMember("name")->value.GetString();
		_assetDefs.push_back(ad);
	}
	
	return true;
}



std::string AssetLoader::getAssetDirPath()
{
	return _assetDirPath;
}



std::string AssetLoader::getLevelDirPath()
{
	return _levelsDirPath;
}
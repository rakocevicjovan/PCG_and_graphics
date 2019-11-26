#include "AssetLoader.h"
#include "FileUtilities.h"



AssetLoader::AssetLoader() {}



bool AssetLoader::loadLevel(const std::string& levelPath)
{
	rapidjson::Document sceneDef;
	sceneDef.Parse(FileUtils::loadFileContents(_projectPath + levelPath).c_str());

	if (!sceneDef.IsObject())
		return false;
	
	if(!loadLevelDef(sceneDef))
		return false;
	
	if (!loadAssetDefs(sceneDef))
		return false;

	for (auto ad : _assetDefs)
		if (!loadAsset(ad))
			return false;

	return true;
}



bool AssetLoader::loadLevelDef(const rapidjson::Document& levelDef)
{
	//THANK YOU WINDOWS, THANKS SO MUCH WOWWWWW
#pragma push_macro("GetObject")
#undef GetObject

	_ld.id = levelDef.FindMember("id")->value.GetInt();
	_ld.name = levelDef.FindMember("name")->value.GetString();
	_ld.description = levelDef.FindMember("description")->value.GetString();
	_ld.projName = levelDef.FindMember("project")->value.GetString();
	_ld.jsonPath = levelDef.FindMember("jsonDefPath")->value.GetString();
	_ld.createdAt = levelDef.FindMember("createdAt")->value.GetString();
	_ld.updatedAt = levelDef.FindMember("updatedAt")->value.GetString();

	return true;

#pragma pop_macro("MACRONAME")
}



bool AssetLoader::loadAssetDefs(const rapidjson::Document& levelDef)
{
	if (!levelDef.FindMember("assets")->value.IsArray())
		return false;

	auto assDefArr = levelDef.FindMember("assets")->value.GetArray();

	AssetDef ad;
	for (rapidjson::Value::ConstValueIterator itr = assDefArr.Begin(); itr != assDefArr.End(); ++itr)
	{
		ad.id = itr->FindMember("id")->value.GetInt();
		ad.path = itr->FindMember("path")->value.GetString();
		ad.name = itr->FindMember("name")->value.GetString();
		_assetDefs.push_back(ad);
	}
	return true;
}



bool AssetLoader::loadAsset(const AssetDef& ad)
{
	return true;
}



void AssetLoader::setProjectData(const std::string & projectPath)	//, const std::vector<std::string>& levelPaths
{
	_projectPath = projectPath;
}
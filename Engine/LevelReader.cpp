#include "pch.h"
#include "LevelReader.h"
#include <rapidjson/document.h>
#include "FileUtilities.h"
#include "Mesh.h"
#include "Model.h"



bool LevelReader::loadLevel(const std::string& levelPath)
{
	rapidjson::Document levelDef;
	levelDef.Parse(FileUtils::loadFileContents(_projectPath + levelPath).c_str());

	if (!levelDef.IsObject())
		return false;
	
	if(!loadLevelDef(levelDef))
		return false;
	
	if (!loadResourceDefs(levelDef))
		return false;

	return true;
}



bool LevelReader::loadLevelDef(const rapidjson::Document& levelDef)
{
#pragma push_macro("GetObject")	//THANK YOU WINDOWS, THANKS SO MUCH WOWWWWW
#undef GetObject

	_ld.id = levelDef.FindMember("id")->value.GetInt();
	_ld.name = levelDef.FindMember("name")->value.GetString();
	_ld.description = levelDef.FindMember("description")->value.GetString();
	_ld.projName = levelDef.FindMember("project")->value.GetString();
	_ld.jsonPath = levelDef.FindMember("jsonDefPath")->value.GetString();
	_ld.createdAt = levelDef.FindMember("createdAt")->value.GetString();
	_ld.updatedAt = levelDef.FindMember("updatedAt")->value.GetString();

	return true;

#pragma pop_macro("GetObject")
}



bool LevelReader::loadResourceDefs(const rapidjson::Document& levelDef)
{
	if (!levelDef.FindMember("assets")->value.IsArray())
		return false;

	auto assDefArr = levelDef.FindMember("assets")->value.GetArray();

	for (rapidjson::Value::ConstValueIterator itr = assDefArr.Begin(); itr != assDefArr.End(); ++itr)
		_resourceDefs.push_back(LoadResourceDef(itr));

	return true;
}



ResourceDef LevelReader::LoadResourceDef(rapidjson::Value::ConstValueIterator itr)
{
	ResourceDef rd;

	rd.key._ID = itr->FindMember("id")->value.GetInt();
	rd.key._assetName = itr->FindMember("name")->value.GetString();

	rd.val._path = itr->FindMember("path")->value.GetString();
	rd.val._resType = ResourceDef::getResTypeFromString(itr->FindMember("resType")->value.GetString());

	return rd;
}



void LevelReader::setProjectPath(const std::string & projectPath)
{
	_projectPath = projectPath;
}



const std::vector<ResourceDef>& LevelReader::getLevelResourceDefs()
{
	return _resourceDefs;
}



void LevelReader::clearLevelResourceDefs()
{
	_resourceDefs.clear();
}
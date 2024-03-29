#include "pch.h"
#include "Project.h"
#include "FileUtilities.h"


bool Project::loadFromConfig(const std::string& projConfPath)
{
	_configPath = projConfPath;

	rapidjson::Document projConfDoc;

	projConfDoc.Parse(FileUtils::loadFileContents(_configPath).c_str());

	if (!projConfDoc.IsObject())
		return false;

	return loadProjectConfiguration(projConfDoc);
}


bool Project::loadProjectConfiguration(const rapidjson::Document& projConfDoc)
{
	// Load a bunch of variables relevant to the current project from the json file
	_projDef._ID = projConfDoc.FindMember("id")->value.GetInt();
	_projDef._projectName = projConfDoc.FindMember("name")->value.GetString();
	_projDef._projectPath = projConfDoc.FindMember("folderPath")->value.GetString();
	_projDef._description = projConfDoc.FindMember("description")->value.GetString();
	_projDef._createdAt = projConfDoc.FindMember("createdAt")->value.GetString();
	_projDef._updatedAt = projConfDoc.FindMember("updatedAt")->value.GetString();
	_projDef._numLevels = projConfDoc.FindMember("levelCount")->value.GetInt();

	return loadLevelList(projConfDoc); //streams release on exit regardless
}


bool Project::loadLevelList(const rapidjson::Document& projConfDoc)
{
	if (!projConfDoc.FindMember("levels")->value.IsArray())
		return false;

	auto levelDefArr = projConfDoc.FindMember("levels")->value.GetArray();

	_projDef._levelList.reserve(_projDef._numLevels);

	for (rapidjson::Value::ConstValueIterator itr = levelDefArr.Begin(); itr != levelDefArr.End(); ++itr)
	{
		_projDef._levelList.emplace_back(itr->FindMember("jsonDefPath")->value.GetString());
	}

	_levelReader.setProjectPath(getProjDir());

	return true;
}
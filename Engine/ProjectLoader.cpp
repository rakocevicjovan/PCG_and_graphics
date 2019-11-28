#include "ProjectLoader.h"
#include "FileUtilities.h"



ProjectLoader::ProjectLoader() {}



bool ProjectLoader::loadProjectConfiguration(const rapidjson::Document& projConfDoc)
{
	//load a bunch of variables relevant to the current project from the json file
	_projDef.id			  = projConfDoc.FindMember("id")->value.GetInt();
	_projDef._projectName = projConfDoc.FindMember("name")->value.GetString();
	_projDef._projectPath = projConfDoc.FindMember("folderPath")->value.GetString();
	_projDef._description = projConfDoc.FindMember("description")->value.GetString();
	_projDef._createdAt	  = projConfDoc.FindMember("createdAt")->value.GetString();
	_projDef._updatedAt	  = projConfDoc.FindMember("updatedAt")->value.GetString();
	_projDef.numLevels	  = projConfDoc.FindMember("levelCount")->value.GetInt();
	
	return loadLevelList(projConfDoc); //streams release on exit regardless
}



bool ProjectLoader::loadLevelList(const rapidjson::Document& projConfDoc)
{
	if (!projConfDoc.FindMember("levels")->value.IsArray())
		return false;

	auto levelDefArr = projConfDoc.FindMember("levels")->value.GetArray();

	_projDef._levelList.reserve(_projDef.numLevels);

	for (rapidjson::Value::ConstValueIterator itr = levelDefArr.Begin(); itr != levelDefArr.End(); ++itr)
	{
		_projDef._levelList.emplace_back(itr->FindMember("jsonDefPath")->value.GetString());
	}

	return true;
}



bool ProjectLoader::loadProjFromConfig(const std::string& projConfPath)
{
	_projConfPath = projConfPath;

	rapidjson::Document projConfDoc;
	projConfDoc.Parse(FileUtils::loadFileContents(_projConfPath).c_str());

	if (!projConfDoc.IsObject())
		return false;

	return loadProjectConfiguration(projConfDoc);
}



const std::string& ProjectLoader::getProjDir() const
{
	return _projDef._projectPath;
}



const ProjectDefinition& ProjectLoader::getProjDef() const
{
	return _projDef;
}



const std::vector<std::string>& ProjectLoader::getLevelList() const
{
	return _projDef._levelList;
}



/* olf signature... prefer ref to rvalue anyways
bool ProjectLoader::loadProjFromConfig(const std::string&& projConfPath)
{
	_projConfPath = projConfPath;

	rapidjson::Document projConfDoc;
	projConfDoc.Parse(FileUtils::loadFileContents(_projConfPath).c_str());

	if (!projConfDoc.IsObject())
		return false;

	return loadProjectConfiguration(projConfDoc);
}
*/



/* old way to load the level list
bool ProjectLoader::loadLevelList()
{
	DIR *dir;
	struct dirent *entry;
	if ((dir = opendir((_projDef._projectPath + LEVEL_SUFFIX).c_str())) != NULL)
	{
		// print all the files and directories within directory
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
	// could not open directory
	perror("Could not open directory");
	return false;
	}
}
*/
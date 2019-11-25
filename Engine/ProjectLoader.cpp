#include "ProjectLoader.h"
#include "rapidjson/document.h"
#include <fstream>
#include <sstream>


ProjectLoader::ProjectLoader()
{
	
}



bool ProjectLoader::loadConfig(const std::string&& projConfPath)
{
	_projConfPath = projConfPath;
	rapidjson::Document projConfDoc;

	std::ifstream t(_projConfPath);
	std::stringstream buffer;
	buffer << t.rdbuf();

	projConfDoc.Parse(buffer.str().c_str());

	//load a bunch of variables relevant to the current project from the json file
	if (projConfDoc.IsObject())
	{
		_pc.id			 = projConfDoc.FindMember("id")->value.GetInt();
		_pc._projectName = projConfDoc.FindMember("name")->value.GetString();
		_pc._description = projConfDoc.FindMember("description")->value.GetString();
		_pc._projectPath = projConfDoc.FindMember("folderPath")->value.GetString();
		_pc._createdAt	 = projConfDoc.FindMember("createdAt")->value.GetString();
		_pc._updatedAt	 = projConfDoc.FindMember("updatedAt")->value.GetString();
		return true;
	}
	
	return false; //streams release on exit regardless
}
#include "ProjectLoader.h"



ProjectLoader::ProjectLoader(const std::string& projConfPath)
{
	_projConfPath = projConfPath;
	//load a bunch of variables relevant to the project, from the json
}



std::string ProjectLoader::getProjConfpath()
{
	return _projConfPath;
}
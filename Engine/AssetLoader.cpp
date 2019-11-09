#include "AssetLoader.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


AssetLoader::AssetLoader()
{
	rapidjson::Document projConfDoc;


	//load base asset folders from the project config file
	//std::string _modelDirPath = 
	//std::string _materialDirPath = 
}



void AssetLoader::loadScene(const std::string& scenePath)
{

}



std::string AssetLoader::getModelDirPath()
{
	return _modelDirPath;
}



std::string AssetLoader::getMaterialDirPath()
{
	return _materialDirPath;
}
#include "ResourceManager.h"

ResourceManager::ResourceManager()// : _stackAllocator(1024 * 1024 * 100)
{
}


ResourceManager::~ResourceManager()
{
}



void ResourceManager::init()
{
	//loads the project configuration data into the project loader, as well as a list of levels associated to the project
	_projLoader.loadProjFromConfig("C:/Users/Senpai/source/repos/PCG_and_graphics_stale_memes/Tower Defense/Tower defense.json");
	_levelReader.setProjectData(_projLoader.getProjDir());

	_stackAllocator.init(1024 * 1024 * 100);
}



void ResourceManager::pushLevel(int i)
{
	_levelReader.loadLevel(_projLoader.getLevelList()[i]);

	//add to memory... which I have yet to implement... but that's down to memory management classes to handle

	const std::vector<ResourceDef>& resDefs = _levelReader.getLevelResourceDefs();
	std::vector<Resource*> resources;
	resources.reserve(resDefs.size());

	for (int i = 0; i < resDefs.size(); ++i)
	{
		//resources.emplace_back(resDefs[i].path, resDefs[i].name);
		if (resDefs[i].type == ResType::MESH)
		{
			resources.emplace_back(new (_stackAllocator.alloc(sizeof(Mesh))) Mesh());
		}
		else if (resDefs[i].type == ResType::TEXTURE)
		{

		}
		
	}

}



void ResourceManager::popLevel(int i)
{

}
#include "ResourceManager.h"

ResourceManager::ResourceManager()
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
}



void ResourceManager::pushLevel(int i)
{
	_levelReader.loadLevel(_projLoader.getLevelList()[i]);

	//add to stack... which I have yet to implement

	auto resDefs = _levelReader.getLevelResourceDefs();

	for (int i = 0; i < resDefs.size(); ++i)
	{
		switch (resDefs[i].type)
		{
		case ResType::MESH:
			break;

		case ResType::TEXTURE:
			break;

		case ResType::MATERIAL:
			break;
		};
	}
}



void ResourceManager::popLevel(int i)
{

}
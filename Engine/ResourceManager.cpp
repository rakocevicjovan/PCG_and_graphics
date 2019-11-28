#include "ResourceManager.h"

ResourceManager::ResourceManager()// : _stackAllocator(1024 * 1024 * 100)
{
}


ResourceManager::~ResourceManager()
{
}



void ResourceManager::init(ID3D11Device* device)
{
	_device = device;

	//loads the project configuration data into the project loader, as well as a list of levels associated to the project
	_projLoader.loadProjFromConfig("C:/Users/Senpai/source/repos/PCG_and_graphics_stale_memes/Tower Defense/Tower defense.json");
	_levelReader.setProjectData(_projLoader.getProjDir());

	_stackAllocator.init(1024 * 1024 * 100);
}



void ResourceManager::pushLevel(int i)
{
	_levelReader.loadLevel(_projLoader.getLevelList()[i]);

	const std::vector<ResourceDef>& resDefs = _levelReader.getLevelResourceDefs();
	std::vector<Resource*> resources;
	resources.reserve(resDefs.size());

	//@TODO check if already loaded by path... all of this is very confusing for now but basically 
	//there is a need to separate ref counting and actual allocation...

	for (int i = 0; i < resDefs.size(); ++i)
	{
		if (resDefs[i].type == ResType::MESH)
		{
			resources.emplace_back(new (_stackAllocator.getHead()) Model());
			resources.back()->setPathName(resDefs[i].path, resDefs[i].name);
			resources.back()->incRef();
			static_cast<Model*>(resources.back())->LoadModel(_device, resDefs[i].path);
			_stackAllocator.alloc(sizeof(Model));
		}
		else if (resDefs[i].type == ResType::TEXTURE)
		{

		}
		
	}

}



void ResourceManager::popLevel(int i)
{

}
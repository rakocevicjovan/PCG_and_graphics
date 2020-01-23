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
	_levelReader.setProjectPath(_projLoader.getProjDir());

	_stackAllocator.init(1024 * 1024 * 30);
}



void ResourceManager::pushLevel(int i)
{
	_levelReader.loadLevel(_projLoader.getLevelList()[i]);

	const std::vector<ResourceDef>& resDefs = _levelReader.getLevelResourceDefs();
	_resourceMap.reserve(resDefs.size());

	//@TODO check if already loaded by path... all of this is very confusing for now but basically 
	//there is a need to separate ref counting and actual allocation...


	for (int i = 0; i < resDefs.size(); ++i)
	{
		auto uMapItr = _resourceMap.find(resDefs[i].name);

		//handle duplicates
		if (uMapItr != _resourceMap.end())
		{
			uMapItr->second->incRef();
			continue;
		}

		if (resDefs[i].type == ResType::MESH)
		{
			Resource* temp = new (_stackAllocator.getHeadPtr()) Model();
			temp->setPathName(resDefs[i].path, resDefs[i].name);
			temp->incRef();
			static_cast<Model*>(temp)->LoadModel(_device, _projLoader.getProjDir() + resDefs[i].path);

			_resourceMap.insert(std::make_pair<>(resDefs[i].name, temp));
			_stackAllocator.alloc(sizeof(Model));
		}
		else if (resDefs[i].type == ResType::TEXTURE)
		{
			Resource *temp = new (_stackAllocator.getHeadPtr()) Texture(_projLoader.getProjDir() + resDefs[i].path);
			temp->setPathName(resDefs[i].path, resDefs[i].name);
			temp->incRef();
			static_cast<Texture*>(temp)->Setup(_device);

			_resourceMap.insert(std::make_pair<>(resDefs[i].name, temp));
			_stackAllocator.alloc(sizeof(Texture));
		}
		
	}

}



void ResourceManager::popLevel(int i)
{
	//but if I clear the stack they will get overwritten... how to get around keeping some of them while using my own allocator
	for (auto resPtr : _resourceMap)
	{
		if (!resPtr.second->isInUse())
		{
			delete resPtr.second;
			resPtr.second = nullptr;
		}
	}
}
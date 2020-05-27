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



void ResourceManager::loadLevel(int i)
{
	_levelReader.loadLevel(_projLoader.getLevelList()[i]);

	const std::vector<ResourceDef>& resDefs = _levelReader.getLevelResourceDefs();
	_resourceMap.reserve(resDefs.size());

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
			Resource* temp = new (_stackAllocator.alloc(sizeof(Model))) Model();
			temp->setPathName(resDefs[i].path, resDefs[i].name);
			temp->incRef();
			static_cast<Model*>(temp)->LoadModel(_device, _projLoader.getProjDir() + resDefs[i].path);
			_resourceMap.insert(std::make_pair<>(resDefs[i].name, temp));
		}
		else if (resDefs[i].type == ResType::TEXTURE)
		{
			Resource *temp = new (_stackAllocator.alloc(sizeof(Texture))) Texture(_projLoader.getProjDir() + resDefs[i].path);
			temp->setPathName(resDefs[i].path, resDefs[i].name);
			temp->incRef();
			static_cast<Texture*>(temp)->SetUpAsResource(_device);
			_resourceMap.insert(std::make_pair<>(resDefs[i].name, temp));
		}
		
	}

}



void ResourceManager::popLevel(int i)
{
	//but if I clear the stack they will get deleted indiscriminately... unsuitable
	for (auto resPtr : _resourceMap)
	{
		if (!resPtr.second->isInUse())
		{
			delete resPtr.second;
			resPtr.second = nullptr;
		}
	}
}
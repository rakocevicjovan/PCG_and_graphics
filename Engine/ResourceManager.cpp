#include "ResourceManager.h"



ResourceManager::ResourceManager() {}



ResourceManager::~ResourceManager() {}



void ResourceManager::init(ID3D11Device* device)
{
	_device = device;

	_stackAllocator.init(1024 * 1024 * 30);
}



void ResourceManager::loadResourceLedger()
{

}



void ResourceManager::loadBatch(const std::string& projDir, const std::vector<ResourceDef>& resDefs)
{
	_resourceMap.reserve(resDefs.size());

	for (int i = 0; i < resDefs.size(); ++i)
	{
		auto uMapItr = _resourceMap.find(resDefs[i]._assetName);

		//handle duplicates
		if (uMapItr != _resourceMap.end())
		{
			uMapItr->second->incRef();
			continue;
		}

		if (resDefs[i]._resType == ResType::MESH)
		{
			Resource* temp = new (_stackAllocator.alloc(sizeof(Model))) Model();
			temp->incRef();
			static_cast<Model*>(temp)->LoadModel(_device, projDir + resDefs[i]._path);
			_resourceMap.insert(std::make_pair<>(resDefs[i]._assetName, temp));
		}
		else if (resDefs[i]._resType == ResType::TEXTURE)
		{
			Resource *temp = new (_stackAllocator.alloc(sizeof(Texture))) Texture(projDir + resDefs[i]._path);
			temp->incRef();
			static_cast<Texture*>(temp)->SetUpAsResource(_device);
			_resourceMap.insert(std::make_pair<>(resDefs[i]._assetName, temp));
		}
	}
}



void ResourceManager::popLevel(UINT levelID)
{
	//but if I clear the stack they will get deleted indiscriminately... unsuitable
	_resourceMap.clear();
}
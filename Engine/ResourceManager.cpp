#include "ResourceManager.h"



ResourceManager::ResourceManager() {}



ResourceManager::~ResourceManager() {}



void ResourceManager::init(ID3D11Device* device)
{
	_device = device;

	_stackAllocator.init(1024 * 1024 * 30);
}



void ResourceManager::loadBatch(const std::string& projDir, const std::vector<ResourceDef>& resDefs)
{
	_resourceMap.reserve(resDefs.size());

	/* Think about this later
		auto uMapItr = _resourceMap.find(resDefs[i].key._assetName);

		// Handle duplicates
		if (uMapItr != _resourceMap.end())
		{
			uMapItr->second->incRef();
			continue;
		}
	*/

	for (int i = 0; i < resDefs.size(); ++i)
	{
		if (resDefs[i].val._resType == ResType::MODEL)
		{
			Resource* temp = new (_stackAllocator.alloc(sizeof(Model))) Model();
			temp->incRef();
			static_cast<Model*>(temp)->loadFromAssimp(_device, projDir + resDefs[i].val._path);
			_resourceMap.insert(std::make_pair<>(resDefs[i].key._assetName, temp));
		}
		else if (resDefs[i].val._resType == ResType::TEXTURE)
		{
			Resource *temp = new (_stackAllocator.alloc(sizeof(Texture))) Texture(projDir + resDefs[i].val._path);
			temp->incRef();
			static_cast<Texture*>(temp)->SetUpAsResource(_device);
			_resourceMap.insert(std::make_pair<>(resDefs[i].key._assetName, temp));
		}
	}
}



void ResourceManager::popLevel(UINT levelID)
{
	//but if I clear the stack they will get deleted indiscriminately... unsuitable
	_resourceMap.clear();
}
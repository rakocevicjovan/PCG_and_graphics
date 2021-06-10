#include "pch.h"
#include "ResourceManager.h"
#include "ModelImporter.h"


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

	for (int i = 0; i < resDefs.size(); ++i)
	{
		if (resDefs[i]._type == ResType::MODEL)
		{
			// This is not stack allocated any more but this code is going to go bye bye at some point so I don't care. It's also insignificant to stack allocate this tbh.
			Model* model = ModelImporter::StandaloneModelImport(_device, projDir + resDefs[i]._path).model.release();
			_resourceMap.insert(std::make_pair<>(resDefs[i]._path, model));
		}
		else if (resDefs[i]._type == ResType::TEXTURE)
		{
			Resource *temp = new (_stackAllocator.alloc(sizeof(Texture))) Texture(projDir + resDefs[i]._path);
			temp->incRef();
			static_cast<Texture*>(temp)->setUpAsResource(_device);
			_resourceMap.insert(std::make_pair<>(resDefs[i]._path, temp));
		}
	}
}



void ResourceManager::unloadBatch(UINT levelID)
{
	//but if I clear the stack they will get deleted indiscriminately... unsuitable
	_resourceMap.clear();
}
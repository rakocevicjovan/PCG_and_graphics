#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

#include "Resource.h"
#include "Texture.h"
#include "Model.h"
#include "Audio.h"
#include "LevelReader.h"
#include "AssetLedger.h"
#include "StackAllocator.h"


//intended for a level-based game... not going to do open world yet until I understand how to implement streaming well

class ResourceManager
{
	AssetLedger _assetLedger;
	StackAllocator _stackAllocator;
	ID3D11Device* _device;
	std::unordered_map<std::string, std::unique_ptr<Resource>> _resourceMap;

public:

	ResourceManager();
	~ResourceManager();

	void init(ID3D11Device*);
	void loadBatch(const std::string& projDir, const std::vector<ResourceDef>&);
	void popLevel(UINT levelID);

	void loadAssetLedger(const std::string& path){
		_assetLedger.load(path);}

	void saveAssetLedger(const std::string& path)
	{
		_assetLedger.save(path);
	}

	template <typename ResType>
	ResType* getByName(const std::string& name)
	{
		return static_cast<ResType*>(_resourceMap[name].get());
	}
};
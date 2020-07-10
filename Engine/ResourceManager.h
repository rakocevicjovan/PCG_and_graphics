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
#include "ResourceLedger.h"
#include "StackAllocator.h"


//intended for a level-based game... not going to do open world yet until I understand how to implement streaming well

class ResourceManager
{
	ResourceLedger resourceLedger;
	StackAllocator _stackAllocator;
	ID3D11Device* _device;
	std::unordered_map<std::string, std::unique_ptr<Resource>> _resourceMap;

public:

	ResourceManager();
	~ResourceManager();

	void init(ID3D11Device*);
	void loadBatch(const std::string& projDir, const std::vector<ResourceDef>&);
	void popLevel(UINT levelID);
	void loadResourceLedger(std::string& path);

	template <typename ResType>
	ResType* getByName(const std::string& name)
	{
		return static_cast<ResType*>(_resourceMap[name].get());
	}
};
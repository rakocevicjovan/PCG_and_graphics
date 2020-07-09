#pragma once
#include <vector>
#include <unordered_map>
#include <memory>

#include "Resource.h"
#include "Texture.h"
#include "Model.h"
#include "Audio.h"
#include "LevelReader.h"
#include "Project.h"
#include "StackAllocator.h"

//intended for a level-based game... not going to do open world yet until I understand how to implement streaming well

class ResourceManager
{
	Project _project;
	StackAllocator _stackAllocator;
	ID3D11Device* _device;
	std::unordered_map<std::string, std::unique_ptr<Resource>> _resourceMap;

public:
	ResourceManager();
	~ResourceManager();

	void init(ID3D11Device*);
	void loadLevel(UINT levelID);
	void popLevel(UINT levelID);

	Project& getProject() { return _project; }

	template <typename ResType>
	ResType* getByName(const std::string& name)
	{
		return static_cast<ResType*>(_resourceMap[name].get());
	}
};
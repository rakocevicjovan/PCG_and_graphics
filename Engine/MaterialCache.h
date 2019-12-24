#pragma once
#include "ShaderCache.h"
#include "Material.h"
#include "ResourceManager.h"
#include <unordered_map>

class MaterialCache
{
private:

	ShaderCache* _shCache;
	ResourceManager* _resMan;

	std::unordered_map<std::string, Material*> _matMap;

public:
	void init(ShaderCache* shaderCache, ResourceManager* resMan);

	Material* getMaterial(const std::string& name);

	bool addMaterial(const std::string& name, Material* mat);

	//@TODO obviously, this is to be replaced, sooner the better
	void createAllMaterialsBecauseIAmTooLazyToMakeThisDataDriven();
};
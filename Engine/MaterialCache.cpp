#include "MaterialCache.h"



void MaterialCache::init(ShaderCache* shaderCache, ResourceManager* resMan)
{
	_shCache = shaderCache;
	_resMan = resMan;
}


//currently, i need to access the model in order to load a texture... this blows. Don't do it!
//textures must exist separately... a good data driven loading system would clear this up a lot
void MaterialCache::createAllMaterialsBecauseIAmTooLazyToMakeThisDataDriven()
{
	Material* floorMat = new Material();
	floorMat->_opaque = true;
	floorMat->setVS(_shCache->getVertShader("basicVS"));
	floorMat->setPS(_shCache->getPixShader("phongPS"));
	addMaterial("floorMat", floorMat);

	Material* skybox = new Material();
	skybox->_opaque = true;
	skybox->setVS(_shCache->getVertShader("skyboxVS"));
	skybox->setPS(_shCache->getPixShader("skyboxPS"));
	addMaterial("skybox", skybox);
}



Material* MaterialCache::getMaterial(const std::string& name)
{
	std::unordered_map<std::string, Material*>::const_iterator found = _matMap.find(name);
	if (found == _matMap.end())
		return nullptr;
	else
		return found->second;
}



bool MaterialCache::addMaterial(const std::string& name, Material* mat)
{
	return _matMap.insert(std::unordered_map<std::string, Material*>::value_type(name, mat)).second;
}
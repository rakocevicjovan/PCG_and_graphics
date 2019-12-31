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
	Material* creepMat = new Material();
	creepMat->opaque = true;
	creepMat->setVS(_shCache->getVertShader("basicVS"));
	creepMat->setPS(_shCache->getPixShader("lightPS"));

	//this kind of crap is exactly why I need a data driven material system... nothing else will cut it!
	creepMat->textures.push_back(&_resMan->getByName<Model*>("FlyingMage")->meshes[0].textures[0]);

	addMaterial("creepMat", creepMat);
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
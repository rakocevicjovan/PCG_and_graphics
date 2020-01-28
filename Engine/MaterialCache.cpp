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
	//models create their own materials on load best they can with assimp...
	//but they don't have a way to know about which shaders to use etc... need to patch them instead of creating like this!
	/*
	Material* phong = new Material();
	phong->opaque = true;
	phong->setVS(_shCache->getVertShader("basicVS"));
	phong->setPS(_shCache->getPixShader("phongPS"));

	//needs to pass a pair now since the system changed
	phong->textures.push_back(&_resMan->getByName<Model>("FlyingMage")->meshes[0].textures[0]);

	addMaterial("phong", phong);
	*/

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
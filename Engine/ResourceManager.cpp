#include "ResourceManager.h"



ResourceManager::ResourceManager()
{
}


ResourceManager::~ResourceManager()
{
}



void ResourceManager::load(ID3D11Device* device)
{
	_device = device;
	_level.init(device);
}

#pragma once
#include "Light.h"
#include "PoolAllocator.h"
#include <vector>
#include <unordered_map>


// This class assumes that there won't be too many lights added and removed often - it stores data optimized for iteration
// It would be prudent to create a pool allocator in case of numerous insertion/removal use case
class LightManager
{
private:

	// For fast iteration every frame, we want them contiguous... this usually won't be that many lights, especially directional
	// but it should be able to handle a lot of point/spot lights in the scene regardless
	std::vector<DLight> _dirLights;
	std::vector<PLight> _pLights;
	std::vector<SLight> _sLights;

	//PoolAllocator<DLight> _padl; and  : _padl(maxDirLights)  harcore option in case i need it

public:

	LightManager() {}
	~LightManager() {}

	LightManager(uint16_t maxDirLights, uint16_t maxPointLights, uint16_t maxSpotLights)
	{
		_dirLights.reserve(maxDirLights);
		_pLights.reserve(maxPointLights);
		_sLights.reserve(maxSpotLights);

		// Evidently, a lot of these fit into the cache very easily... even my mid range i5 has 256 kb l1 cache
		sizeof(DLight);	// 8192 in cache
		sizeof(PLight);	// same
		sizeof(SLight); // 5461, although it's not aligned but oh well
	}



	//For accessing specific lights by handle
	std::unordered_map<UINT, SceneLight*> _lightMap;

	UINT addDirLight(const DLight& dl)
	{
		_dirLights.push_back(dl);
		return _dirLights.size() - 1;
	}


	void removeDirLight(UINT index)
	{
		std::swap(_dirLights[index], _dirLights.back());	//invalidates index of last dir light since it moved!!!
		_dirLights.pop_back();

		_lightMap.at(_dirLights.size()) = &_dirLights[index];
	}



	void addPointLight(const PLight& pl)
	{
		_pLights.push_back(pl);
	}



	void addSpotLight(const SLight& sl)
	{
		_sLights.push_back(sl);
	}




};
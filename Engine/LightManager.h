#pragma once
#include "Light.h"
#include "PoolAllocator.h"
#include <list>


// This class assumes that there won't be too many lights added and removed often - it stores data optimized for iteration
// It would be prudent to use a pool allocated version for different use cases, which I don't need right now
class LightManager
{
private:

	// For fast iteration every frame, we want them contiguous... this usually won't be that many lights, especially directional
	// but it should be able to handle a lot of point/spot lights in the scene regardless
	std::list<DLight*> _dirLights;
	PoolAllocator<DLight> _dlPool;

	std::list<PLight*> _pLights;
	PoolAllocator<PLight> _plPool;

	std::list<SLight*> _sLights;
	PoolAllocator<SLight> _slPool;


public:

	LightManager(uint16_t maxDirLights, uint16_t maxPointLights, uint16_t maxSpotLights)
		: _dlPool(maxDirLights), _plPool(maxPointLights), _slPool(maxSpotLights)
	{
		//_dirLights.reserve(maxDirLights);
		//_pLights.reserve(maxPointLights);
		//_sLights.reserve(maxSpotLights);

		// Evidently, a lot of these fit into the cache very easily... even my mid range i5 has 256 kb l1 cache
		sizeof(DLight);
		sizeof(PLight);
		sizeof(SLight);
	}

	~LightManager() {}


	DLight* addDirLight(const DLight& dl)
	{
		DLight* dlPtr = new (_dlPool.allocate()) DLight(dl);
		_dirLights.push_back(dlPtr);
		return dlPtr;
	}

	void removeDirLight(DLight* dLight)
	{
		_dlPool.deallocate(dLight);
	}



	PLight* addPointLight(const PLight& pl)
	{
		PLight* plPtr = new (_plPool.allocate()) PLight(pl);
		_pLights.push_back(plPtr);
		return plPtr;
	}

	void removePointLight(PLight* pLight)
	{
		_plPool.deallocate(pLight);
	}



	SLight* addSpotLight(const SLight& sl)
	{
		SLight* slPtr = new (_slPool.allocate()) SLight(sl);
		_sLights.push_back(slPtr);
		return slPtr;
	}

	void removePointLight(SLight* sLight)
	{
		_slPool.deallocate(sLight);
	}

};
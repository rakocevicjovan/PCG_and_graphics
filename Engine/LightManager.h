#pragma once
#include "Light.h"
#include "PoolAllocator.h"
#include "Frustum.h"
#include "ColFuncs.h"
#include <list>


struct PlPoolStruct
{
	PLight _pl;
	PLight* _next;
};

struct SlPoolStruct
{
	SLight _sl;
	SLight* _next;
};


// This class assumes that there won't be too many lights added and removed often - it stores data optimized for iteration
// Probably should eliminate list inserts if we need those operations often
class LightManager
{
private:

	// @TODO replace these with the above structs so that list is baked into the pool allocator
	// because this way we still cause dynamic allocations, and avoiding them was the whole point of using pool allocators
	std::list<DLight*> _dirLights;
	std::list<PLight*> _pLights;
	std::list<SLight*> _sLights;

	// For fast iteration every frame, we want them contiguous... this usually won't be that many lights, especially directional
	// but it should be able to handle a lot of point/spot lights in the scene regardless and keep stable references

	PoolAllocator<DLight> _dlPool;
	PoolAllocator<PLight> _plPool;
	PoolAllocator<SLight> _slPool;

	std::vector<PLight> _culledPLs;
	std::vector<SLight> _culledSLs;



public:

	// Last two values are not too important, light manager will reserve that many slots for culled point/spot lights per frame
	// and the implementation lets that grow
	LightManager(uint16_t maxDirLights, uint16_t maxPointLights, uint16_t maxSpotLights, uint16_t frPLights, uint16_t frSLights) :
		_dlPool(maxDirLights),
		_plPool(maxPointLights),
		_slPool(maxSpotLights)
	{
		_culledPLs.reserve(frPLights);
		_culledSLs.reserve(frPLights);

		sizeof(PlPoolStruct);
		sizeof(SlPoolStruct);
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



	// Could be better to store pointers to them instead of copying them but it costs an indirection + not contiguous, depends...
	void cullLights(const Frustum& frustum)
	{
		// @TODO redo collision functions to take the bare minimum data instead of SphereHull/cone structs... this is wasteful!
		for (const PLight* p : _pLights)
		{
			if (Col::FrustumSphereIntersection(frustum, SphereHull(p->_posRange)))
			{
				//memcpy(_culledPLs.alloc(sizeof(PLight)), p, sizeof(PLight));	No need for a stack allocator, complicates things
				_culledPLs.push_back(*p);
			}
		}

		for (const SLight* s : _sLights)
		{
			if(Col::FrustumConeIntersection(frustum, Cone(s->_posRange, SVec3(s->_dirCosTheta), s->_radius)))
			{
				//memcpy(_culledSLs.alloc(sizeof(SLight)), s, sizeof(SLight));
				_culledSLs.push_back(*s);
			}
		}
	}



	inline const std::vector<PLight>& getCulledPointLights() const
	{
		return _culledPLs;		//return reinterpret_cast<PLight*>(_culledPLs.getStackPtr());
	}



	inline const std::vector<SLight>& getCulledSpotLights() const
	{
		return _culledSLs;
	}



	inline void resetPerFramePools()
	{
		_culledPLs.clear();
		_culledSLs.clear();
	}
};
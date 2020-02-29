#pragma once
#include "Light.h"
#include <vector>
#include <unordered_map>


// This class assumes that there won't be too many lights added and removed often - it stores data optimized for iteration
class LightManager
{
	LightManager() {}
	~LightManager() {}

	// For fast iteration every frame, we want them contiguous... this usually won't be that many lights, especially for directional lights
	// but it should be able to handle a lot of point/spot lights in the scene regardless
	std::vector<DLight> _dirLights;
	std::vector<PLight> _pLights;
	std::vector<SLight> _sLights;

	//for accessing specific lights without searching linearly
	std::unordered_map<UINT, SceneLight*> _lightMap;

	void addDirLight(const DLight& dl)
	{
		_dirLights.push_back(dl);
		_lightMap.insert(std::make_pair(_dirLights.size(), &_dirLights.back()));
	}

	void addPointLight(const PLight& pl)
	{
		_pLights.push_back(pl);
	}

	void addSLight(const SLight& sl)
	{
		_sLights.push_back(sl);
	}
};
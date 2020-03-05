#pragma once
#include "GameObject.h"
#include "LightManager.h"

class Scene
{
private:

	LightManager* _lightManager;

public:

	Scene()
	{
		_lightManager = new LightManager(4, 256, 256, 128, 128);
	}

	~Scene()
	{
		delete _lightManager;
	}



	// Terrain chunks, lights, meshes, cameras... you name it! Master list, will probably separate into several lists instead
	std::vector<GameObject*> _objects;

	// Acceleration structures (octrees/quadtrees)

	// Scene graph - purely for concatenating transforms, might be implicit (within entities themselves) instead of a big graph object

	// tbd...
};
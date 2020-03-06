#pragma once
#include "GameObject.h"
#include "LightManager.h"
#include "Octree.h"

class Scene
{
private:

	LightManager* _lightManager;

	// Terrain chunks, lights, meshes, cameras... you name it! Master list, will probably separate into several lists instead
	std::vector<GameObject*> _objects;

	Octree _oct;

	// Acceleration structures (octrees/quadtrees)

	// Scene graph - purely for concatenating transforms, might be implicit (within entities themselves) instead of a big graph object

	// tbd...

public:

	Scene()
	{
		_lightManager = new LightManager(4, 256, 256, 128, 128);
	}

	~Scene()
	{
		delete _lightManager;
	}


	void assignLightsToObjects(Camera& c)
	{
		_lightManager->cullLights(c._frustum);

		PLight* p = _lightManager->getVisiblePointLightArray();
		for (int i = 0; i < _lightManager->getVisiblePointLightCount(); i++)
		{
			PLight cpl = p[i];
			//insert into octree, cull, assign...
			_oct.insertObject(&SphereHull(cpl._posRange));
		}


		_lightManager->resetFramePools();
	}

};
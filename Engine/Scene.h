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
	std::vector<Actor*> _actors;

	std::vector<Actor*> _visibleActors;

	Octree _oct;

	// Acceleration structures (octrees/quadtrees)

	// Scene graph - purely for concatenating transforms, might be implicit (within entities themselves) instead of one big graph

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

		std::vector<Actor*> litObjects;
		litObjects.reserve(100);

		PLight* p = _lightManager->getVisiblePointLightArray();
		for (int i = 0; i < _lightManager->getVisiblePointLightCount(); i++)
		{
			PLight cpl = p[i];
			// Query octree, cull, assign...
			_oct.findWithin(SVec3(cpl._posRange), cpl._posRange.w, litObjects);
			
			for (Actor* a : litObjects)
			{
				//a.addLight()
			}

			litObjects.clear();
		}

		_lightManager->resetPerFramePools();
	}



	void frustumCull(const Camera& cam)
	{
		_visibleActors.clear();

		for (size_t i = 0; i < _actors.size(); ++i)
		{
			Actor* a = _actors[i];
			SphereHull* sph = static_cast<SphereHull*>(a->getBoundingHull());

			if (Col::FrustumSphereIntersection(cam._frustum, *sph))
			{
				_visibleActors.push_back(a);
			}
		}
	}

};
#pragma once
#include "GameObject.h"
#include "LightManager.h"
#include "Octree.h"

class Scene
{
private:

	UINT numCulled = 0u;

	LightManager* _lightManager;

	// Terrain chunks, lights, meshes, cameras... you name it! Master list, will probably separate into several lists instead
	std::vector<GameObject*> _objects;
	
	std::vector<Renderable> _renderables;

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


		_lightManager->resetFramePools();
	}



	void frustumCull(const Camera& cam)
	{
		numCulled = 0;
		const SMatrix v = cam.GetViewMatrix();
		const SVec3 v3c(v._13, v._23, v._33);
		const SVec3 camPos = cam.GetPosition();

		for (size_t i = 0; i < _renderables.size(); ++i)
		{
			/*if (Col::FrustumSphereIntersection(cam._frustum, ))
				_creeps[i].renderables[j].zDepth = (_creeps[i].transform.Translation() - camPos).Dot(v3c);
			else
				numCulled++;*/
		}
	}

};
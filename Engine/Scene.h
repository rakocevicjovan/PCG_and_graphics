#pragma once
#include "GameObject.h"
#include "LightManager.h"
#include "Octree.h"

#define DEFAULT_SUBDIV_LEVELS 4u

class Scene
{
private:

	// Terrain chunks, lights, meshes, cameras... you name it! Master list, will probably separate into several lists instead
	std::vector<GameObject*> _objects;
	std::vector<Actor*> _actors;

	std::vector<Actor*> _visibleActors;



	std::vector<Actor*> _litObjectPool;

	// Acceleration structures (octrees/quadtrees)

	// Scene graph - purely for concatenating transforms, might be implicit (within entities themselves) instead of one big graph

	// tbd...

public:

	LightManager* _lightManager;
	Octree _octree;



	Scene(const AABB& scope, UINT subdivLevels = DEFAULT_SUBDIV_LEVELS) : _octree(scope, subdivLevels)
	{
		_lightManager = new LightManager(4, 256, 256, 128, 128);
		_litObjectPool.reserve(20);

		// Octree with depth 5 is really big, probably not worth it for my game
		_octree.preallocateRootOnly();	//_oct.preallocateTree();	
	}



	~Scene()
	{
		delete _lightManager;
	}



	void update()
	{
		//@TODO redo this, does redundant work and blows in general
		_octree.updateAll();	

		// Pool allocation speeds this up a lot, also empties are only deallocated once per frame, not bad.
		_octree.lazyTrim();

		// Works, but I need to change the way collision response is handled
		_octree.collideAll();
	}



	void draw()
	{

	}



	void illuminate(Camera& c)
	{
		// Obtain a list of visible lights, store it in the light manager
		_lightManager->cullLights(c._frustum);

		// For every point light
		PLight* p = _lightManager->getVisiblePointLightArray();
		for (int i = 0; i < _lightManager->getVisiblePointLightCount(); i++)
		{
			PLight cpl = p[i];

			// Obtain the list of actors lit by this point light
			_octree.findWithin(SVec3(cpl._posRange), cpl._posRange.w, _litObjectPool);	// Searches the whole tree... seems unnecessary!

			for (Actor*& a : _litObjectPool)
				a->_pLights.push_back(&cpl);	// Seems bad but it will resize to practical max soon, plus usually should be small

			_litObjectPool.clear();	// Reset the list
		}

		
		// For every spot light
		SLight* s = _lightManager->getVisibleSpotLightArray();
		for (int i = 0; i < _lightManager->getVisiblePointLightCount(); i++)
		{
			SLight csl = s[i];

			// Obtain the list of actors lit by this spot light
			// @TODO MAKE IT USE THE CONE TEST NOT SPHERE THIS GIVES FALSE POSITIVES
			_octree.findWithin(SVec3(csl._posRange), csl._posRange.w, _litObjectPool);

			for (Actor*& a : _litObjectPool)
				a->_sLights.push_back(&csl);

			_litObjectPool.clear();	// Reset the list
		}

	}



	void frustumCull(const Camera& cam)
	{
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



	void frameCleanUp()
	{
		_lightManager->resetPerFramePools();
		_visibleActors.clear();
	}

};
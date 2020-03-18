#pragma once
#include "GameObject.h"
#include "LightManager.h"
#include "Octree.h"
#include "Renderer.h"

#define DEFAULT_SUBDIV_LEVELS 4u

class Scene
{
private:
	Renderer& _renderer;

	// Terrain chunks, lights, meshes, cameras... you name it! Master list, will probably separate into several lists instead
	std::vector<GameObject*> _objects;

	std::vector<Actor*> _visibleActors;

	std::vector<Actor*> _litObjectPool;

	// Acceleration structures (octrees/quadtrees)

	// Scene graph - purely for concatenating transforms, might be implicit (within entities themselves) instead of one big graph

	// tbd...

public:

	LightManager* _lightManager;
	Octree _octree;
	std::vector<Actor*> _actors;

	UINT _numCulled;


	Scene(
		Renderer& r,
		const AABB& scope, 
		UINT subdivLevels = DEFAULT_SUBDIV_LEVELS) 
		:
		_renderer(r),
		_octree(scope, subdivLevels),
		_numCulled(0u)
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
		_renderer.d3d()->ClearColourDepthBuffers();
		_renderer.d3d()->setRSSolidNoCull();
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
		_numCulled = 0;

		const SMatrix v = cam.GetViewMatrix();
		const SVec3 viewForward(v._13, v._23, v._33);
		const SVec3 camPos = cam.GetPosition();

		for (size_t i = 0; i < _actors.size(); ++i)
		{
			Actor* a = _actors[i];

			for (size_t j = 0; j < a->_renderables.size(); ++j)
			{
				SphereHull* sph = static_cast<SphereHull*>(a->getBoundingHull(j));

				if (Col::FrustumSphereIntersection(cam._frustum, *sph))
				{
					a->_renderables[j].zDepth = (a->transform.Translation() - camPos).Dot(viewForward);	//@TODO
					_renderer.addToRenderQueue(a->_renderables[j]);
				}
				else
				{
					++_numCulled;
				}
			}
			
		}
	}



	void frameCleanUp()
	{
		_lightManager->resetPerFramePools();
		_visibleActors.clear();
	}

};
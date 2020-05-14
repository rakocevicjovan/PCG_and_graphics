#pragma once
#include "GameObject.h"
#include "LightManager.h"
#include "Octree.h"
#include "Renderer.h"
#include "CSM.h"
#include "ComputeShader.h"
#include <memory>

#define DEFAULT_SUBDIV_LEVELS 4u

class Scene
{
private:
	Renderer& _renderer;
	ShaderCache& _shCache;
	MaterialCache& _matCache;
	Systems& _sys;

	// Terrain chunks, lights, meshes, cameras... you name it! Master list, will probably separate into several lists instead
	std::vector<GameObject*> _objects;

	std::vector<Actor*> _visibleActors;
	std::vector<Actor*> _shadowVisibleActors;

	std::vector<Actor*> _litObjectPool;

	// Acceleration structures (octrees/quadtrees)

	// Scene graph - purely for concatenating transforms, might be implicit (within entities themselves) instead of one big graph

	// tbd...

public:

	std::unique_ptr<LightManager> _lightManager;

	Octree _octree;
	UINT _numCulled;

	CSM _csm;

	std::vector<Actor*> _actors;

	Scene(
		Systems& sys,
		const AABB& scope, 
		UINT subdivLevels = DEFAULT_SUBDIV_LEVELS) 
		:
		_renderer(sys._renderer),
		_shCache(sys._shaderCache),
		_matCache(sys._matCache),
		_sys(sys),
		_octree(scope, subdivLevels),
		_numCulled(0u)
	{
		
		_lightManager = std::make_unique<LightManager>(4, 256, 256, 128, 128);
		_litObjectPool.reserve(20);

		_octree.preallocateRootOnly();
	}



	void update()
	{
		//@TODO redo this, does redundant work and blows in general
		//_octree.updateAll();	

		// Don't do it from within the tree, clear and reinsert... profile if this is better

		_octree.clear();

		for (Actor*& a : _actors)
		{
			for(Hull* h : a->_collider.getHulls())
				_octree.insertObject(static_cast<SphereHull*>(h));
		}

		// Pool allocation speeds this up a lot, also empties are only deallocated once per frame, not bad.
		_octree.lazyTrim();

		// Works, but I need to change the way collision response is handled
		_octree.collideAll();
	}



	void draw()
	{
		frustumCullScene(_renderer._cam);

		illuminate(_renderer._cam);

		_renderer.d3d()->ClearColourDepthBuffers();		//_renderer.d3d()->setRSSolidNoCull();

		// CSM code
		SMatrix dlViewMatrix = DirectX::XMMatrixLookAtLH(SVec3(0, 1000, 0), SVec3(0, 0, 0), SVec3(0, 0, 1));
		_csm.calcProjMats(_renderer._cam, dlViewMatrix);

		_csm.beginShadowPassSequence(_renderer.context());

		for (int i = 0; i < _csm.getNMaps(); ++i)
		{
			_csm.beginShadowPassN(_renderer.context(), i);

			// add floor renderable to the actor list instead

			frustumCull(_csm.getNthFrustum(i), _visibleActors, _shadowVisibleActors);

			for (Actor*& actor : _shadowVisibleActors)
				_csm.drawToCurrentShadowPass(_renderer.context(), actor->_renderables[0]);

			_shadowVisibleActors.clear();
		}

		_renderer.setDefaultRenderTarget();

		// After the shadow maps have been rendered to, we bind the global csm buffer and texture array
		_csm.uploadCSMBuffer(_renderer.context(), PS_CSM_CBUFFER_REGISTER);
		_renderer.context()->PSSetShaderResources(PS_CSM_TEXTURE_REGISTER, 1, _csm.getResView());

		_renderer.sortRenderQueue();
		_renderer.flushRenderQueue();
		_renderer.clearRenderQueue();

		_csm.unbindTextureArray(_renderer.context());

		frameCleanUp();
	}



	void frustumCullScene(const Camera& cam)
	{
		const SMatrix v = cam.GetViewMatrix();
		const SVec3 viewForward(v._13, v._23, v._33);
		const SVec3 camPos = cam.GetPosition();

		frustumCull(cam._frustum, _actors, _visibleActors);

		for (Actor*& a : _visibleActors)
		{
			a->addToRenderQueue(_renderer, camPos, viewForward);
		}

		_numCulled = _actors.size() - _visibleActors.size();
	}



	void frustumCull(const Frustum& frustum, const std::vector<Actor*>& all, std::vector<Actor*>& culled)
	{
		for (Actor* a : _actors)
		{
			for (Hull* sph : a->_collider.getHulls())
			{
				if (Col::FrustumSphereIntersection(frustum, *static_cast<SphereHull*>(sph)))
				{
					culled.push_back(a);
					break;	// Don't insert multiple times, if either hull is inside, object counts as inside
				}
			}
		}
	}



	void illuminate(Camera& c)
	{
		// Obtain a list of visible lights, which will remain stored in the light manager
		_lightManager->cullLights(c._frustum);

		// For every point light
		auto culledPLs = _lightManager->getCulledPointLights();

		_renderer._clusterManager->assignLights(culledPLs, c, _sys._threadPool);
		_renderer._clusterManager->upload(S_CONTEXT, culledPLs);


		// For every spot light
		// Well, implement spot light culling eventually...

	}



	void Scene::cluster(ComputeShader& cs)
	{

	}



	void frameCleanUp()
	{
		_visibleActors.clear();
		_lightManager->resetPerFramePools();
	}

};
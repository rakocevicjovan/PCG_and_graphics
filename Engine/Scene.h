#pragma once
#include "GameObject.h"
#include "LightManager.h"
#include "Octree.h"
#include "Renderer.h"
#include "CSM.h"

class Engine;

class Scene
{
private:
	Renderer& _renderer;
	Engine& _sys;

	// Terrain chunks, lights, meshes, cameras... you name it! Master list, will probably separate into several lists instead
	std::vector<GameObject*> _objects;

	// Temporary data created and discarded per frame
	std::vector<Actor*> _visibleActors;
	std::vector<Actor*> _shadowVisibleActors;

	// Acceleration structures (octrees/quadtrees)

	// Scene graph - purely for concatenating transforms, might be implicit (within entities themselves) instead of one big graph

	// tbd...

public:

	std::unique_ptr<LightManager> _lightManager;

	entt::registry _registry;

	Octree _octree;
	UINT _numCulled;

	CSM<4u> _csm;

	std::vector<Actor*> _actors;


	Scene(Engine& sys, const AABB& scope, UINT subdivLevels);

	void update();

	void draw();

	void frustumCullScene(const Camera& cam);

	void frustumCull(const Frustum& frustum, const std::vector<Actor*>& all, std::vector<Actor*>& culled);

	void illuminate(const Camera& c);

	void frameCleanUp();

	const Camera& getActiveCamera();
};
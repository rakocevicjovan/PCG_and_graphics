#pragma once
#include "Level.h"
#include "Math.h"
#include "Model.h"
#include "Light.h"
#include "SceneGraph.h"
#include "Octree.h"
#include "NavGrid.h"
#include "AStar.h"

class TDLevel : public Level
{
public:
	TDLevel(Systems& sys) : Level(sys) {};

	void init(Systems& sys) override;
	void update(const RenderContext& rc) override;
	void draw(const RenderContext& rc) override;
	void demolish() { this->~TDLevel(); };

private:

	SceneGraph _sg;
	Octree _oct;

	//some enemy specific stuff...
	UINT NUM_ENEMIES = 100u;
	float FLYING_HEIGHT = 10.f;

	//unlike loaded models in the manager, these will be generated
	Procedural::Terrain terrain;
	Model floorModel;

	NavGrid _navGrid;

	PointLight pLight;
	
	CubeMapper skyboxCubeMapper;

	std::vector<Actor> creeps;

	SRay ray;
	int numCulled;

	Material creepMat;

	//octree meshes
	Model debugModel;
	std::vector<AABB> tempBoxes;
	std::vector<InstanceData> octNodeMatrices;
};
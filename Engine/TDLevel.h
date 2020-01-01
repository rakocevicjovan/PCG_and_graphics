#pragma once
#include "Level.h"
#include "Math.h"
#include "Model.h"
#include "Light.h"
#include "SceneGraph.h"
#include "Octree.h"
#include "NavGrid.h"
#include "AStar.h"
#include "TDController.h"	//#include "RTSController.h" flawed idea...
#include "TDGUI.h"



class TDLevel : public Level
{
public:
	TDLevel(Systems& sys) : Level(sys) {};

	void init(Systems& sys) override;
	void update(const RenderContext& rc) override;
	void draw(const RenderContext& rc) override;
	void demolish();

private:

	void rayPick(Camera* cam);
	void handleInput();

	SceneGraph _sg;
	Octree _octree;

	//some enemy specific stuff...
	UINT NUM_ENEMIES = 100u;
	float FLYING_HEIGHT = 10.f;

	//unlike loaded models in the manager, these will be generated
	Procedural::Terrain terrain;
	Model floorModel;
	NavGrid _navGrid;
	UINT GOAL_INDEX = 0;

	PointLight pLight;
	
	CubeMapper skyboxCubeMapper;

	std::vector<Actor> creeps;

	int numCulled;

	//octree meshes
	Model debugModel;
	std::vector<AABB> tempBoxes;
	std::vector<InstanceData> octNodeMatrices;

	TDController _tdController;
	std::vector<Actor> _built;
	Actor tower;
	bool _building = false;
	TDGUI _tdgui;
};
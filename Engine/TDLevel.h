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
#include "Economy.h"
#include "Building.h"
#include "Enemy.h"



class TDLevel : public Level
{
public:
	TDLevel(Systems& sys) : Level(sys) {};

	void init(Systems& sys) override;
	void update(const RenderContext& rc) override;
	void draw(const RenderContext& rc) override;
	void demolish();

private:

	//should really be in scene manager
	void cull(const RenderContext& rc);

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

	std::vector<Enemy> creeps;

	int numCulled;

	//octree meshes
	Model debugModel;
	std::vector<AABB> tempBoxes;
	std::vector<InstanceData> octNodeMatrices;


	//gameplay
	void selectBuildingToBuild(Building* b);
	void addBuildable(Actor&& a, const std::string& name, BuildingType type, const BuildingGuiDef& guiDef);
	void rayPickTerrain(const Camera* cam);
	Building* rayPickBuildings(const Camera* cam);
	void handleInput(const Camera* cam);	//involves ray picking, so we need this
	void steerEnemies(float dTime);


	TDController _tdController;
	TDGUI _tdgui;

	Building* _templateBuilding = nullptr;
	Building* _selectedBuilding = nullptr;
	bool _inBuildingMode = false;
	std::vector<Building*> _buildable;
	std::list<Building> _structures;
	
	Economy _eco;
};
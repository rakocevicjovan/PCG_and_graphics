#pragma once
#include "Level.h"
#include "Math.h"
#include "Model.h"
#include "Light.h"
#include "Scene.h"
#include "Octree.h"
#include "NavGrid.h"
#include "AStar.h"
#include "GUI.h"

//game specific
#include "TDController.h"
#include "TDGUI.h"
#include "Economy.h"
#include "IndustrialBuilding.h"
#include "Tower.h"
#include "Enemy.h"

//tests
//#include "QuadTree.h"
//#include "PoolAllocator.h"

#define DEBUG_OCTREE_NOT

class TDLevel : public Level
{
public:
	TDLevel(Systems& sys);

	void init(Systems& sys) override;
	void update(const RenderContext& rc) override;
	void draw(const RenderContext& rc) override;
	void freeLevelMemory();

private:

	Scene _scene;

	// Level specific
	UINT NUM_ENEMIES = 100u;
	float FLYING_HEIGHT = 10.f;

	// Unlike models loaded using the manager, these will be generated procedurally
	
	float _tSize = 500.f;
	Procedural::Terrain terrain;
	Mesh floorMesh;
	Renderable floorRenderable;
	Renderable frustumRenderable;

	// Navigation
	NavGrid _navGrid;
	UINT GOAL_INDEX = 0;

	// Lights
	PointLight pLight;
	DirectionalLight dirLight;

	//Model globe;
	//Renderable globeRenderable;



	// Gameplay functionality, a lot of this should be in TDGame class (that doesn't even exist reeeee)
	void addBuildables();
	void fixBuildable(Building* b);
	void build();
	void demolish();
	void rayPickTerrain(const Camera* cam);
	Building* rayPickBuildings(const Camera* cam);
	void handleInput(const Camera* cam);	//involves ray picking, so we need this
	void steerEnemies(float dTime);
	float resolveAttack(const Attack& att, const Armour& arm);


	// Gameplay variables
	TDController _tdController;
	GUI _gui;
	TDGUI _tdgui;

	Building* _templateBuilding = nullptr;	//not owning
	Building* _selectedBuilding = nullptr;	//not owning
	bool _inBuildingMode = false;

	// Types of buildings that can be built
	std::vector<Building*> _buildable;
	
	//@TODO replace all three with SlotVector or similar
	std::list<Tower> _towers;					// List of towers (martial buildings)
	std::list<IndustrialBuilding> _industry;	// List of industrial buildings
	std::list<Building*> _structures;			// Master list for easier access (coding-vise) cba writing two loops
	// The master list type-slices but it doesn't matter, it's just there to use it AS a base class - example, rendering
	
	// List of enemies in the wave
	std::vector<Enemy> _creeps;
	
	// Class that manages in-game resources, fairly simple
	Economy _eco;

#ifdef DEBUG_OCTREE

	struct InstanceData { SMatrix transform; };

	//for header
	Model debugModel;
	Actor debugActor;
	std::vector<AABB> tempBoxes;
	std::vector<SMatrix> octNodeMatrices;
#endif
};
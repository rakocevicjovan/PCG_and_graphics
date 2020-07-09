#pragma once
#include "Level.h"
#include "Math.h"
#include "Model.h"
#include "Light.h"
#include "Scene.h"
#include "NavGrid.h"
#include "AStar.h"
#include "Skybox.h"
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
#include "Editor.h"
#include "LoaderGUI.h"



class TDLevel : public Level
{
public:
	TDLevel(Engine& sys);

	void init(Engine& sys) override;
	void update(const RenderContext& rc) override;
	void draw(const RenderContext& rc) override;
	void freeLevelMemory();

private:

	// Scene data
	Scene _scene;
	Procedural::Terrain terrain;
	Skybox _skybox;

	// Enemy data
	UINT NUM_ENEMIES = 100u;
	float FLYING_HEIGHT = 10.f;
	
	// Level navigation data
	NavGrid _navGrid;
	UINT GOAL_INDEX = 0;

	PointLight pLight;
	DirectionalLight dirLight;

	// Things that need changing
	Mesh floorMesh;
	Renderable floorRenderable;
	Actor terrainActor;
	Actor debugSphereActor;
	Renderable frustumRenderable;

	Model testModel;

	//Model globe;
	//Renderable globeRenderable;

	// Gameplay functionality - belongs in other classes (such as game class)
	void addBuildables();
	void fixBuildable(Building* b);
	void build();
	void demolish();
	void rayPickTerrain(const Camera* cam);
	Building* rayPickBuildings(const Camera* cam);
	void handleInput(const Camera* cam);	//involves ray picking, so we need this
	void steerEnemies(float dTime);
	float resolveAttack(const Attack& att, const Armour& arm);


	TDController _tdController;
	TDGUI _tdgui;

	Building* _templateBuilding = nullptr;	//not owning
	Building* _selectedBuilding = nullptr;	//not owning
	bool _inBuildingMode = false;


	// Palette of possible buildings available to the player
	std::vector<Building*> _buildable;

	// Use pool allocator/vector? Should improve iteration but neither will ever get too big anyways so not important
	std::list<Tower> _towers;
	std::list<IndustrialBuilding> _industry;

	// Master list, type-slices but it doesn't matter, it's just there to use it AS a base class - example, rendering
	std::list<Building*> _structures;

	// Vector of enemies in the current wave
	std::vector<Enemy> _creeps;
	
	// Simple economy class
	Economy _eco;

	Editor _editor;
	LoaderGUI _loaderGui;
	//AssimpPreview _loader;

#define DEBUG_OCTREE_NOT

#ifdef DEBUG_OCTREE

	struct InstanceData { SMatrix transform; };

	//for header
	Model debugModel;
	Actor debugActor;
	std::vector<AABB> tempBoxes;
	std::vector<SMatrix> octNodeMatrices;
#endif
};
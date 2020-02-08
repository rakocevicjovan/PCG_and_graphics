#pragma once
#include "Level.h"
#include "Math.h"
#include "Model.h"
#include "Light.h"
#include "SceneGraph.h"
#include "Octree.h"
#include "NavGrid.h"
#include "AStar.h"
#include "Skybox.h"

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
#include "CSM.h"



class TDLevel : public Level
{
public:
	TDLevel(Systems& sys) : Level(sys) {};

	void init(Systems& sys) override;
	void update(const RenderContext& rc) override;
	void draw(const RenderContext& rc) override;
	void freeLevelMemory();

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
	Mesh floorMesh;
	Renderable floorRenderable;
	NavGrid _navGrid;
	UINT GOAL_INDEX = 0;

	PointLight pLight;
	DirectionalLight dirLight;
	
	//CubeMapper skyboxCubeMapper;
	Skybox _skybox;

	int numCulled;

	Model globe;
	Renderable globeRenderable;

	CSM _csm;

	//gameplay
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


	std::vector<Building*> _buildable;
	//@TODO replace all three with SlotVector

	std::list<Tower> _towers;
	std::list<IndustrialBuilding> _industry;
	std::list<Building*> _structures;
	//the master list type-slices but it doesn't matter, it's just there to use it AS a base class - example, rendering
	

	//changes per wave
	std::vector<Enemy> _creeps;
	
	Economy _eco;
};


/*

void Renderer::renderSkybox(const Camera& cam, Model& skybox, const CubeMapper& skyboxCubeMapper)
{
	_d3d->setRSSolidNoCull();
	_d3d->SwitchDepthToLessEquals();
	skyboxShader.SetShaderParameters(_deviceContext, cam, rc.dTime, skyboxCubeMapper.cm_srv);
	skybox.Draw(_deviceContext, _shMan.skyboxShader);
	skyboxShader.ReleaseShaderParameters(_deviceContext);
	_d3d->SwitchDepthToDefault();
	_d3d->setRSSolidCull();
}

*/
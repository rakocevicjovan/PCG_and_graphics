#include "pch.h"
#include "TDLevel.h"
#include "Terrain.h"
#include "Geometry.h"
#include "AStar.h"
#include "Picker.h"
#include "ColFuncs.h"
#include "Shader.h"
#include "Steering.h"

#include "PooledSceneGraph.h"
#include "entt/entt.hpp"


inline float pureDijkstra(const NavNode& n1, const NavNode& n2) { return 0.f; }


TDLevel::TDLevel(Engine& sys) 
	: Level(sys), _scene(_sys, AABB(SVec3(), SVec3(500.f * .5)), 5), _geoClipMap(3, 4, 10.)
{
};



///INIT AND HELPERS
void TDLevel::init(Engine& sys)
{
	//ShaderGenerator shg(_sys._shaderCompiler);	shg.mix();

	_geoClipMap.init(S_DEVICE);

	_sceneEditor.init(&_scene);

	/* Load everything up for the level. Preserve order of these functions three */
	//_sys._resMan.loadBatch(PROJ.getProjDir(), PROJ.getLevelReader().getLevelResourceDefs());	// This actually is data driven :)
	_sys._shaderCache.createAllShadersBecauseIAmTooLazyToMakeThisDataDriven(&sys._shaderCompiler);
	//_sys._matCache.createAllMaterialsBecauseIAmTooLazyToMakeThisDataDriven();

	Model* skyBoxModel = S_RESMAN.getByName<Model>("Skysphere");

	_scene._csm.init(S_DEVICE, 1024u, 1024u, S_SHCACHE.getVertShader("csmVS"));

	S_INMAN.registerController(&_tdController);

	//_skybox = Skybox(S_DEVICE, "../Textures/day.dds", skyBoxModel, S_MATCACHE.getMaterial("skybox"));

	_tdgui.init(ImVec2(S_WW - 500, S_WH - 300), ImVec2(500, 300));
	_tdgui.createWidget(ImVec2(0, S_WH - 300), ImVec2(300, 300), "selected");

	// Light setup, to be replaced soon
	LightData lightData(SVec3(0.1, 0.7, 0.9), .03f, SVec3(0.8, 0.8, 1.0), .2, SVec3(0.3, 0.5, 1.0), 0.7);
	
	_pLight = PointLight(lightData, SVec4(0, 300, 300, 1));
	_pLight.createCBuffer(S_DEVICE, _pLightCB);
	_pLight.updateCBuffer(S_CONTEXT, _pLightCB);

	_dirLight = DirectionalLight(lightData, SVec4(0, -1, 0, 0));
	_dirLight.createCBuffer(S_DEVICE, _dirLightCB);
	_dirLight.updateCBuffer(S_CONTEXT, _dirLightCB);

	float _tSize = 500.f;
	terrain = Procedural::Terrain(2, 2, SVec3(_tSize));
	terrain.setOffset(-_tSize * .5f, -0.f, -_tSize * .5f);
	terrain.SetUp(S_DEVICE);

	
	// Generate the floor gemetry... really simple but a lot of material fuss afterwards
	floorMesh = Mesh(terrain, S_DEVICE);

	Texture floorTex(nullptr, "../Textures/LavaIntense/diffuse.jpg");
	floorMesh._material->_materialTextures.push_back({ {TextureRole::DIFFUSE}, std::make_shared<Texture>(floorTex) });
	
	floorRenderable = Renderable(floorMesh, SMatrix::CreateTranslation(terrain.getOffset()));
	floorRenderable.mat->setVS(S_SHCACHE.getVertShader("csmSceneVS"));
	floorRenderable.mat->setPS(S_SHCACHE.getPixShader("csmScenePS"));	//clusterPS clusterDebugPS

	terrainActor.addRenderable(floorRenderable, 500);
	//terrainActor._collider.dynamic = false;
	terrainActor._collider.collidable = false;
	//_scene._actors.push_back(&terrainActor);

	// Initialize navigation grid
	_navGrid = NavGrid(10, 10, SVec2(50.f), terrain.getOffset());
	_navGrid.forbidCell(99);
	_navGrid.createAllEdges();
	AStar<pureDijkstra>::fillGraph(_navGrid._cells, _navGrid._edges, GOAL_INDEX);
	_navGrid.setGoalIndex(GOAL_INDEX);
	_navGrid.fillFlowField();

	// Initialize all enemies
	_creeps.reserve(NUM_ENEMIES);

	auto modelPtr = S_RESMAN.getByName<Model>("FlyingMage");
	auto vsPtr = sys._shaderCache.getVertShader("basicVS");
	auto psPtr = sys._shaderCache.getPixShader("phongPS");

	for (int i = 0; i < NUM_ENEMIES; ++i)
	{
		SVec3 pos = SVec3(200, 0, 200) + 5 * SVec3(i % 10, 0, (i / 10) % 10);

		_creeps.emplace_back(
			modelPtr,
			SMatrix::CreateScale(2.f) * SMatrix::CreateTranslation(pos),
			100.f, 50.f, 1.1f, 0.9f
		);

		_creeps[i]._steerComp._mspeed = 40.f;
		//_creeps[i].renderableHandle = _scene.addRenderable() @TODO MAKE IT WORK LIKE THIS!
		
		_creeps[i].patchMaterial(vsPtr, psPtr);

		for (Hull* h : _creeps[i]._collider.getHulls())
			_scene._octree.insertObject(static_cast<SphereHull*>(h));

		_scene._actors.push_back(&(_creeps[i]));
	}

	//Add building types, @TODO make data driven
	addBuildables();

	//Add resource types, @TODO make data driven
	_eco.createResource("Coin", 1000);
	_eco.createResource("Wood", 1000);

	/// DEBUG

	/*
	std::vector<PLight> lightList(16);

	for (int i = 0; i < lightList.size(); ++i)
	{
		SVec3 pos = SVec3(i % 4, .0f, i / 4) * 50.f + SVec3(0, 10., 0.f);
		lightList[i] = PLight(Math::getNormalizedVec3(SVec3(i % 2, (i / 2) % 2, ((16 - i) / 4))), 100., SVec3(&pos.x));
		_scene._lightManager.get()->addPointLight(lightList[i]);
	}

	// Dissect this and similar init-s for assets, it's not tolerable, all needs to go data driven!
	SMatrix dbgSphMat = SMatrix::CreateScale(lightList[0]._posRange.w);
	Math::SetTranslation(dbgSphMat, SVec3(&lightList[0]._posRange.x));

	Renderable dbgRenderable(S_RESMAN.getByName<Model>("Skysphere")->_meshes[0]);	//, _lightList[0]._posRange.w
	dbgRenderable.mat = new Material(sys._shaderCache.getVertShader("basicVS"), sys._shaderCache.getPixShader("phongPS"), true);

	debugSphereActor.addRenderable(dbgRenderable, lightList[0]._posRange.w);
	debugSphereActor._renderables.back()._transform = dbgSphMat;
	//debugSphereActor._renderables[0].mat->_texMetaData.push_back({ &floorMesh._textures.back(), TextureRole::DIFFUSE });
	*/


#ifdef DEBUG_OCTREE
	Procedural::Geometry g1;
	g1.GenBox(SVec3(1));
	debugModel.meshes.push_back(Mesh(g1, S_DEVICE, true, false));
	debugActor = Actor(&debugModel);
	debugActor._renderables[0].mat->setVS(sys._shaderCache.getVertShader("basicVS"));
	debugActor._renderables[0].mat->setPS(sys._shaderCache.getPixShader("phongPS"));
	debugActor._renderables[0].mat->pLight = &pLight;
	tempBoxes.reserve(1000);
	octNodeMatrices.reserve(1000);

	Actor octree;
#endif
}


// Use files for this we aren't savages gdi
void TDLevel::addBuildables()
{
	_buildable.reserve(2);
	Building* b = new Tower(
		Actor(S_RESMAN.getByName<Model>("GuardTower")),
		"Guard tower",
		BuildingType::MARTIAL,
		BuildingGuiDef(
			"Guard tower is a common, yet powerful defensive building.",
			"Guard tower",
			S_RESMAN.getByName<Texture>("guard_tower")->_srv.Get()),
		Attack(100.f, 100.f, Attack::AttackType::PHYS, .5f, 0.f)
	);
	b->patchMaterial(_sys._shaderCache.getVertShader("basicVS"), _sys._shaderCache.getPixShader("phongPS"));
	fixBuildable(b);

	b = new IndustrialBuilding(
		Actor(S_RESMAN.getByName<Model>("Lumberyard")),
		"Lumberyard",
		BuildingType::INDUSTRIAL,
		BuildingGuiDef(
			"Produces 10 wood per minute. Time to get lumber-jacked.",
			"Lumberyard",
			S_RESMAN.getByName<Texture>("lumber_yard")->_srv.Get()),
		Income(10.f, "Coin", 10.f)
	);
	b->patchMaterial(_sys._shaderCache.getVertShader("basicVS"), _sys._shaderCache.getPixShader("phongPS"));
	fixBuildable(b);
}



void TDLevel::fixBuildable(Building* b)
{
	_buildable.push_back(b);

	//hacky workaround but aight for now, replaces default hull(s) with the special one for TD
	_buildable.back()->_collider.deleteAndClearHulls();
	_buildable.back()->_collider.addHull(new SphereHull(SVec3(), 25));
	_buildable.back()->_collider._parent = _buildable.back();

	//can use pointers but this much data replicated is not really important
	_tdgui.addBuildingGuiDef(_buildable.back()->_guiDef);
}



///UPDATE AND HELPERS
void TDLevel::update(const RenderContext& rc)
{
	steerEnemies(rc.dTime);

	_scene.update();

	//moves around the selected building
	if (_templateBuilding && _inBuildingMode)
	{
		rayPickTerrain(rc.cam);
		_templateBuilding->propagate();
	}

	handleInput(rc.cam);

	// Could use octree if that will help it go faster...
	for (Tower& tower : _towers)
	{
		tower.advanceCooldown(rc.dTime);

		if (!tower.readyToFire())	// Exit early per tower, no need to check anything, tower not ready
			continue;

		for (Enemy& creep : _creeps)
		{
			if (creep.isDead())	//exit early per creep with cheapest option
				continue; 

			if (!tower.inRange(creep.getPosition()))	//exit early per creep, less cheap but still all right
				continue;
			
			creep.receiveDamage(resolveAttack(tower.shoot(), creep._arm));

			break;	//we managed to shoot, it will be on CD, just break immediately
		}
	}

#ifdef DEBUG_OCTREE
	_scene._octree.getTreeAsAABBVector(tempBoxes);

	for (int i = 0; i < tempBoxes.size(); ++i)
	{
		octNodeMatrices.push_back(
			(
				SMatrix::CreateScale(tempBoxes[i].getHalfSize() * 2.f) *
				SMatrix::CreateTranslation(tempBoxes[i].getPosition())
				).Transpose()
		);
	}

	tempBoxes.clear();
#endif
}



//really simple for now, don't need more - also this should go to game logic not here...
float TDLevel::resolveAttack(const Attack& att, const Armour& arm)
{
	float result = att._damage;
	float armMultiplier = att._type == Attack::AttackType::PHYS ? arm._physArmour : arm._magicArmour;
	float flatPen = att._flatPen;

	//attack's penetration value simply changes the multiplier, no fancy math needed
	return result * (armMultiplier + flatPen);
}


// Again, not for here...
void TDLevel::rayPickTerrain(const Camera* cam)
{
	MCoords mc = _sys._inputManager.getAbsXY();
	SRay ray = Picker::generateRay(_sys.getWinW(), _sys.getWinH(), mc.x, mc.y, *cam);

	//intersect base plane for now... terrain works using projection + bresenham/superset if gridlike
	SVec3 POI;
	ray.direction *= 500.f;
	Col::RayPlaneIntersection(ray, SVec3(0, 0, 0), SVec3(1, 0, 0), SVec3(0, 0, 1), POI);

	SVec3 snappedPos = _navGrid.snapToCell(POI);
	Math::SetTranslation(_templateBuilding->_transform, snappedPos);
}



void TDLevel::handleInput(const Camera* cam)
{
	//check if the spot is taken - using the nav grid, only clear cells can do! and update the navgrid after

	InputEventTD inEvent;
	
	while (_tdController.consumeNextAction(inEvent))
	{
		switch (inEvent)
		{
		case InputEventTD::SELECT:

			if (_inBuildingMode)
			{
				build();
			}
			else	//select an existing building
			{
				_selectedBuilding = rayPickBuildings(cam);
			}
			break;

		case InputEventTD::STOP_BUILDING:
			_inBuildingMode = false;
			break;

		case InputEventTD::RESET_CREEPS:
			for (int i = 0; i < _creeps.size(); ++i)
			{
				Math::SetTranslation(_creeps[i]._transform, SVec3(200, 0, 200) + 5 * SVec3(i % 10, 0, (i / 10) % 10));
				_creeps[i]._steerComp._active = true;
				_creeps[i].revive();
			}
			break;
		}
	}
}



Building* TDLevel::rayPickBuildings(const Camera* cam)
{
	MCoords mc = _sys._inputManager.getAbsXY();
	SRay ray = Picker::generateRay(_sys.getWinW(), _sys.getWinH(), mc.x, mc.y, *cam);
	ray.direction *= 500.f;

	std::list<SphereHull*> sps;

	Building* b = nullptr;

	_scene._octree.rayCastTree(ray, sps);

	float minDist = 9999999.f;

	SphereHull* closest = nullptr;

	for (SphereHull* s : sps)
	{
		float dist = (s->ctr - cam->GetPosition()).LengthSquared();
		if (dist < minDist)
		{
			closest = s;
			minDist = dist;
		}
	}

	if(closest)
		b = dynamic_cast<Building*>(closest->_collider->_parent);

	return b;
}


//using clone to create new building instances
void TDLevel::build()
{
	if (_navGrid.tryAddObstacle(_templateBuilding->getPosition()))
	{
		AStar<pureDijkstra>::fillGraph(_navGrid._cells, _navGrid._edges, GOAL_INDEX);
		_navGrid.fillFlowField();

		if (_templateBuilding->_type == BuildingType::MARTIAL)
		{
			Tower* t = static_cast<Tower*>(_templateBuilding->clone());
			_towers.push_back(*t);
			delete t;
			_structures.push_back(&_towers.back());
		}
		else
		{
			IndustrialBuilding* ib = static_cast<IndustrialBuilding*>(_templateBuilding->clone());
			_industry.push_back(*ib);
			delete ib;
			_structures.push_back(&_industry.back());
		}

		//_scene._actors.push_back(_structures.back());
		_scene._octree.insertObject((SphereHull*)_structures.back()->_collider.getHull(0));

		_inBuildingMode = false;
		_templateBuilding = nullptr;
	}
	else
	{
		//detected path blocking, can't build, pop some gui warning etc...
	}
	//could use traits to infer the type but it's not really an issue with only two types... what better way?
	//single array idea could make shooting worse with longer iterations...	
}


void TDLevel::demolish()
{
	_navGrid.removeObstacle(_navGrid.posToCellIndex(_selectedBuilding->getPosition()));
	AStar<pureDijkstra>::fillGraph(_navGrid._cells, _navGrid._edges, GOAL_INDEX);
	_navGrid.fillFlowField();

	_scene._octree.removeObject((SphereHull*)_selectedBuilding->_collider.getHull(0));

	// @TODO add a == sign... also this sucks big time its gonna compare them all, best not do it this way
	if (_selectedBuilding->_type == BuildingType::MARTIAL)
		_towers.remove(*static_cast<Tower*>(_selectedBuilding));
	else
		_industry.remove(*static_cast<IndustrialBuilding*>(_selectedBuilding));

	_structures.remove(_selectedBuilding);
	_selectedBuilding = nullptr;
}



void TDLevel::steerEnemies(float dTime)
{
	// Not known to individuals as it depends on group size, therefore should not be in a unit component I'd say... 
	SVec2 stopArea(sqrt(_creeps.size()));
	stopArea *= 9.f;
	float stopDistance = stopArea.Length();

	std::vector<Actor*> neighbourCreepVec;
	neighbourCreepVec.reserve(25);			//max neighbours to consider...

	for (int i = 0; i < _creeps.size(); ++i)
	{
		if (_creeps[i].isDead())
			continue;

		//pathfinding and steering, needs to turn off once nobody is moving...

		if (_creeps[i]._steerComp._active)
		{
			_scene._octree.findWithin(_creeps[i].getPosition(), 5.f, neighbourCreepVec);
			_creeps[i]._steerComp.update(_navGrid, dTime, neighbourCreepVec, i, stopDistance);
			neighbourCreepVec.clear();
		}

		//height
		float h = terrain.getHeightAtPosition(_creeps[i].getPosition());
		float intervalPassed = fmod(_sys._clock.totalTime() * 5.f + i * 2.f, 10.f);
		float sway = intervalPassed < 5.f ? Math::smoothstep(0, 5, intervalPassed) : Math::smoothstep(10, 5, intervalPassed);
		Math::setHeight(_creeps[i]._transform, h + 2 * sway + FLYING_HEIGHT);

		//propagate transforms to children
		_creeps[i].propagate();
	}
}


///DRAW AND HELPERS
void TDLevel::draw(const RenderContext& rc)
{
	if (_inBuildingMode)
	{
		for (Renderable& r : _templateBuilding->_renderables)
			S_RANDY.addToRenderQueue(r);
	}

	for (Building* building : _structures)
	{
		for (Renderable& r : building->_renderables)
			S_RANDY.addToRenderQueue(r);
	}

	_pLight.updateCBuffer(S_CONTEXT, _pLightCB);
	_pLight.bind(S_CONTEXT, _pLightCB);
	//_dirLight.bind(S_CONTEXT, _dirLightCB);

	_scene.draw();

	// Ideally this would draw first but scene is still clunky... will see.
	S_RANDY.d3d()->setRSWireframe();
	_geoClipMap.draw(S_CONTEXT);
	S_RANDY.d3d()->setRSSolidCull();

	_skybox.renderSkybox(*rc.cam, S_RANDY);



#ifdef DEBUG_OCTREE
	//shady.instanced.SetShaderParameters(context, debugModel, *rc.cam, pLight, rc.dTime);
	//debugModel.DrawInstanced(context, shady.instanced);
	//shady.instanced.ReleaseShaderParameters(context);
	for (int i = 0; i < octNodeMatrices.size(); i++)
	{
		debugActor._renderables[0]._transform = octNodeMatrices[i].Transpose();
		S_RANDY.render(debugActor._renderables[0]);
	}
	octNodeMatrices.clear();
#endif


	GUI::beginFrame();

	_sceneEditor.display();
		
	//ID3D11ShaderResourceView** wat = const_cast<ID3D11ShaderResourceView**>(_scene._csm.getResView());
	//ImGui::Image(_scene._csm.getDebugView(), ImVec2(500, 500));	// pls

	_fpsCounter.tickFast(rc.dTime);
	std::vector<GuiElement> guiElems =
	{
		{"Octree",	std::string("OCT node count " + std::to_string(_scene._octree.getNodeCount()))},
		{"Octree",	std::string("OCT hull count " + std::to_string(_scene._octree.getHullCount()))},
		{"FPS",		std::string("FPS: " + std::to_string(_fpsCounter.getAverageFPS()))},
		{"Culling", std::string("Objects culled:" + std::to_string(_scene._numCulled))}
	};
	GUI::renderGuiElems(guiElems);


	UINT structureIndex;
	if (_tdgui.renderBuildingPalette(structureIndex))
	{
		_templateBuilding = _buildable[structureIndex];
		_inBuildingMode = true;
	}


	if (_selectedBuilding != nullptr)
	{
		if (_tdgui.renderSelectedWidget(_selectedBuilding->_guiDef))
		{
			demolish();
		}
	}

	_eco.renderEconomyWidget();
	
	GUI::endFrame();

	rc.d3d->EndScene();
}



///whatever this is... don't need it really
void TDLevel::freeLevelMemory()
{
	finished = true;
}





/// Currently unused, yet useful, code
/* old sphere placement, it's here because it's rad
for (int i = 0; i < 125; ++i)
{
	SVec3 pos = SVec3(i % 5, (i / 5) % 5, (i / 25) % 5) * 20.f + SVec3(5.f);
}*/

/*
	Procedural::Geometry g;
	g.GenBox(SVec3(_navGrid.getCellSize().x, 1, _navGrid.getCellSize().y));
	boxModel.meshes.push_back(Mesh(g, S_DEVICE, true, false));
	box = Actor(SMatrix(), &boxModel);
	box.renderables[0].mat = &creepMat;
	box.renderables[0].pLight = &pLight;
*/

/*
// Create a box mesh for frustum debugging
Procedural::Geometry g;
g.GenBox(SVec3(2., 2., 1.));
for (SVec3& gp : g.positions)
gp += SVec3(0., 0., 0.5);

Mesh* boxMesh = new Mesh(g, S_DEVICE, true, false);
frustumRenderable = Renderable(*boxMesh);
frustumRenderable.mat->pLight = &pLight;
frustumRenderable.mat->setVS(S_SHCACHE.getVertShader("basicVS"));
frustumRenderable.mat->setPS(S_SHCACHE.getPixShader("phongPS"));
*/
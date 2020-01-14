#include "TDLevel.h"
#include "Terrain.h"
#include "Geometry.h"
#include "AStar.h"
#include "Picker.h"
#include "ColFuncs.h"
#include "Shader.h"
#include "Steering.h"

//#define DEBUG_OCTREE


inline float pureDijkstra(const NavNode& n1, const NavNode& n2) { return 0.f; }


///INIT AND HELPERS
void TDLevel::init(Systems& sys)
{
	QuadTree qt;

	qt.create(100.f);

	S_INMAN.registerController(&_tdController);
	skyboxCubeMapper.LoadFromFiles(S_DEVICE, "../Textures/day.dds");

	_tdgui.init(ImVec2(S_WW - 500, S_WH - 300), ImVec2(500, 300));
	_tdgui.createWidget(ImVec2(0, S_WH - 300), ImVec2(300, 300), "selected");

	LightData lightData(SVec3(0.1, 0.7, 0.9), .03f, SVec3(0.8, 0.8, 1.0), .2, SVec3(0.3, 0.5, 1.0), 0.7);
	pLight = PointLight(lightData, SVec4(0, 500, 0, 1));

	float tSize = 500;
	terrain = Procedural::Terrain(2, 2, SVec3(tSize));
	terrain.setOffset(-tSize * .5f, -0.f, -tSize * .5f);
	terrain.SetUp(S_DEVICE);
	floorModel = Model(terrain, S_DEVICE);

	_octree.init(AABB(SVec3(), SVec3(tSize * .5)), 4);	//with depth 5 it's really big, probably not worth it for my game
	_octree.prellocateRootOnly();						//_oct.preallocateTree();	

	_navGrid = NavGrid(10, 10, SVec2(50.f), terrain.getOffset());
	_navGrid.forbidCell(99);
	_navGrid.createAllEdges();
	AStar<pureDijkstra>::fillGraph(_navGrid._cells, _navGrid._edges, GOAL_INDEX);
	_navGrid.setGoalIndex(GOAL_INDEX);
	_navGrid.fillFlowField();

	_creeps.reserve(NUM_ENEMIES);
	for (int i = 0; i < NUM_ENEMIES; ++i)
	{
		SVec3 pos = SVec3(200, 0, 200) + 5 * SVec3(i % 10, 0, (i / 10) % 10);

		_creeps.emplace_back(
			S_RESMAN.getByName<Model>("FlyingMage"), 
			SMatrix::CreateScale(2.f) * SMatrix::CreateTranslation(pos),
			100.f, 50.f, 1.1f, 0.9f
		);

		_creeps[i]._steerComp._mspeed = 40.f;
		
		for (Renderable& r : _creeps[i].renderables)
		{
			r.mat = S_MATCACHE.getMaterial("creepMat");
			r.pLight = &pLight;
		}

		_octree.insertObject(static_cast<SphereHull*>(_creeps[i]._collider.getHull(0)));
	}


	//Add building types... again, could make data driven...
	addBuildables();

	/*for (int i = 0; i < 50; i++)
	{
		_towers.push_back(MartialBuilding(*(MartialBuilding*)_buildable[0]));
	}*/
	

	//Add resource types, same @TODO as above
	_eco.createResource("Coin", 1000);
	_eco.createResource("Wood", 1000);
}



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
			S_RESMAN.getByName<Texture>("guard_tower")->srv),
		Attack(100.f, 100.f, Attack::AttackType::PHYS, .5f, 0.f)
	);
	b->patchMaterial(_sys._shaderCache.getVertShader("basicVS"), _sys._shaderCache.getPixShader("lightPS"), pLight);
	fixBuildable(b);

	b = new IndustrialBuilding(
		Actor(S_RESMAN.getByName<Model>("Lumberyard")),
		"Lumberyard",
		BuildingType::INDUSTRIAL,
		BuildingGuiDef(
			"Produces 10 wood per minute. Time to get lumber-jacked.",
			"Lumberyard",
			S_RESMAN.getByName<Texture>("lumber_yard")->srv),
		Income(10.f, "Coin", 10.f)
	);
	b->patchMaterial(_sys._shaderCache.getVertShader("basicVS"), _sys._shaderCache.getPixShader("lightPS"), pLight);
	fixBuildable(b);
}



void TDLevel::fixBuildable(Building* b)
{
	_buildable.push_back(b);

	//hacky workaround but aight for now, replaces default hull(s) with the special one for TD
	_buildable.back()->_collider.deleteAndClearHulls();
	_buildable.back()->_collider.addHull(new SphereHull(SVec3(), 25));
	_buildable.back()->_collider.parent = _buildable.back();

	//can use pointers but this much data replicated is not really important
	_tdgui.addBuildingGuiDef(_buildable.back()->_guiDef);
}



///UPDATE AND HELPERS
void TDLevel::update(const RenderContext& rc)
{	
	//this works well to reduce the number of checked branches with simple if(null) but only profiling
	//can tell if it's better this way or by just leaving them allocated (which means deeper checks, but less allocations)
	//Another alternative is having a bool empty; in the octnode...
	_octree.updateAll();	//@TODO redo this, does redundant work and blows in general
	_octree.lazyTrim();
	_octree.collideAll();

	steerEnemies(rc.dTime);

	//moves around the selected building
	if (_templateBuilding && _inBuildingMode)
	{
		rayPickTerrain(rc.cam);
		_templateBuilding->propagate();
	}

	handleInput(rc.cam);
	
	//could use octree if that will help it go faster...
	for (Tower& tower : _towers)
	{
		tower.advanceCooldown(rc.dTime);

		if (!tower.readyToFire())	//exit early per tower, no need to check anything, tower not ready
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

	cull(rc);
}



//really simple for now, don't need more
float TDLevel::resolveAttack(const Attack& att, const Armour& arm)
{
	float result = att._damage;
	float armMultiplier = att._type == Attack::AttackType::PHYS ? arm._physArmour : arm._magicArmour;
	float flatPen = att._flatPen;

	//attack's penetration value simply changes the multiplier, no fancy math needed
	return result * (armMultiplier + flatPen);
}



void TDLevel::rayPickTerrain(const Camera* cam)
{
	MCoords mc = _sys._inputManager.getAbsXY();
	SRay ray = Picker::generateRay(_sys.getWinW(), _sys.getWinH(), mc.x, mc.y, *cam);

	//intersect base plane for now... terrain works using projection + bresenham/superset if gridlike
	SVec3 POI;
	ray.direction *= 500.f;
	Col::RayPlaneIntersection(ray, SVec3(0, 0, 0), SVec3(1, 0, 0), SVec3(0, 0, 1), POI);

	SVec3 snappedPos = _navGrid.snapToCell(POI);
	Math::SetTranslation(_templateBuilding->transform, snappedPos);
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
				Math::SetTranslation(_creeps[i].transform, SVec3(200, 0, 200) + 5 * SVec3(i % 10, 0, (i / 10) % 10));
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

	_octree.rayCastTree(ray, sps);

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
		b = dynamic_cast<Building*>(closest->_collider->parent);

	return b;
}


//awful syntax but I want to enforce the copy constructor deep copying the stuff...
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

		_octree.insertObject((SphereHull*)_structures.back()->_collider.getHull(0));

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

	_octree.removeObject((SphereHull*)_selectedBuilding->_collider.getHull(0));

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
	//not known to individuals as it depends on group size, therefore should not be in a unit component I'd say... 
	SVec2 stopArea(sqrt(_creeps.size()));
	stopArea *= 9.f;
	float stopDistance = stopArea.Length();

	for (int i = 0; i < _creeps.size(); ++i)
	{
		if (_creeps[i].isDead())
			continue;

		//pathfinding and steering, needs to turn off once nobody is moving...

		if (_creeps[i]._steerComp._active)
		{
			std::list<Actor*> neighbourCreeps;	//this should be on the per-frame allocator
			_octree.findWithin(_creeps[i].getPosition(), 5.f, neighbourCreeps);
			_creeps[i]._steerComp.update(_navGrid, dTime, neighbourCreeps, i, stopDistance);
		}

		//height
		float h = terrain.getHeightAtPosition(_creeps[i].getPosition());
		float intervalPassed = fmod(_sys._clock.TotalTime() * 5.f + i * 2.f, 10.f);
		float sway = intervalPassed < 5.f ? Math::smoothstep(0, 5, intervalPassed) : Math::smoothstep(10, 5, intervalPassed);
		Math::setHeight(_creeps[i].transform, h + 2 * sway + FLYING_HEIGHT);

		//propagate transforms to children
		_creeps[i].propagate();
	}
}


///DRAW AND HELPERS
void TDLevel::draw(const RenderContext& rc)
{
	rc.d3d->ClearColourDepthBuffers();
	rc.d3d->setRSSolidNoCull();

	S_SHADY.light.SetShaderParameters(S_CONTEXT, floorModel.transform, *rc.cam, pLight, rc.dTime);
	floorModel.Draw(S_CONTEXT, S_SHADY.light);
	S_SHADY.light.ReleaseShaderParameters(S_CONTEXT);

	S_RANDY.sortRenderQueue();
	S_RANDY.flushRenderQueue();
	S_RANDY.clearRenderQueue();

	S_RANDY.renderSkybox(*rc.cam, *(S_RESMAN.getByName<Model>("Skysphere")), skyboxCubeMapper);

	if (_inBuildingMode)
		_templateBuilding->render(S_RANDY);

	for (Building* building : _structures)
		building->render(S_RANDY);

	startGuiFrame();


	std::vector<GuiElement> guiElems =
	{
		{"Octree",	std::string("OCT node count " + std::to_string(_octree.getNodeCount()))},
		{"Octree",	std::string("OCT hull count " + std::to_string(_octree.getHullCount()))},
		{"FPS",		std::string("FPS: " + std::to_string(1 / rc.dTime))},
		{"Culling", std::string("Objects culled:" + std::to_string(numCulled))}
	};
	renderGuiElems(guiElems);


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

	endGuiFrame();

	rc.d3d->EndScene();
}


//cull and add to render queue
void TDLevel::cull(const RenderContext& rc)
{
	numCulled = 0;
	const SMatrix v = rc.cam->GetViewMatrix();
	const SVec3 v3c(v._13, v._23, v._33);
	const SVec3 camPos = rc.cam->GetPosition();

	float zDepth;

	for (int i = 0; i < _creeps.size(); ++i)
	{
		if (Col::FrustumSphereIntersection(rc.cam->frustum, *static_cast<SphereHull*>(_creeps[i]._collider.getHull(0))))
		{
			zDepth = (_creeps[i].transform.Translation() - camPos).Dot(v3c);
			for (auto& r : _creeps[i].renderables)
			{
				r.zDepth = zDepth;
				S_RANDY.addToRenderQueue(r);
			}
		}
		else
		{
			numCulled++;
		}
	}
}


///whatever this is... don't need it really
void TDLevel::freeLevelMemory()
{
	finished = true;
}





///Currently unused, yet useful, code
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


#ifdef DEBUG_OCTREE	//for init()
Procedural::Geometry g;
g.GenBox(SVec3(1));
debugModel.meshes.push_back(Mesh(g, S_DEVICE));
tempBoxes.reserve(1000);
octNodeMatrices.reserve(1000);
#endif


#ifdef DEBUG_OCTREE	//for update()
_oct.getTreeAsAABBVector(tempBoxes);

for (int i = 0; i < tempBoxes.size(); ++i)
{
	octNodeMatrices.push_back(
		(
			SMatrix::CreateScale(tempBoxes[i].getHalfSize() * 2.f) *
			SMatrix::CreateTranslation(tempBoxes[i].getPosition())
			).Transpose()
	);
}

shady.instanced.UpdateInstanceData(octNodeMatrices);
octNodeMatrices.clear();
tempBoxes.clear();
#endif


#ifdef DEBUG_OCTREE
shady.instanced.SetShaderParameters(context, debugModel, *rc.cam, pLight, rc.dTime);
debugModel.DrawInstanced(context, shady.instanced);
shady.instanced.ReleaseShaderParameters(context);
#endif
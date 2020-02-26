#include "TDGame.h"



void TDGame::addBuildables()
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
	b->patchMaterial(_sys._shaderCache.getVertShader("basicVS"), _sys._shaderCache.getPixShader("phongPS"), pLight);
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
	b->patchMaterial(_sys._shaderCache.getVertShader("basicVS"), _sys._shaderCache.getPixShader("phongPS"), pLight);
	fixBuildable(b);
}



void TDGame::fixBuildable(Building* b)
{
	_buildable.push_back(b);

	//hacky workaround but aight for now, replaces default hull(s) with the special one for TD
	_buildable.back()->_collider.deleteAndClearHulls();
	_buildable.back()->_collider.addHull(new SphereHull(SVec3(), 25));
	_buildable.back()->_collider.parent = _buildable.back();

	//can use pointers but this much data replicated is not really important
	_tdgui.addBuildingGuiDef(_buildable.back()->_guiDef);
}



//really simple for now, don't need more
float TDGame::resolveAttack(const Attack& att, const Armour& arm)
{
	float result = att._damage;
	float armMultiplier = att._type == Attack::AttackType::PHYS ? arm._physArmour : arm._magicArmour;
	float flatPen = att._flatPen;

	//attack's penetration value simply changes the multiplier, no fancy math needed
	return result * (armMultiplier + flatPen);
}



void TDGame::rayPickTerrain(const Camera* cam)
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



void TDGame::handleInput(const Camera* cam)
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



Building* TDGame::rayPickBuildings(const Camera* cam)
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

	if (closest)
		b = dynamic_cast<Building*>(closest->_collider->parent);

	return b;
}


//using clone to create new building instances
void TDGame::build()
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


void TDGame::demolish()
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



void TDGame::steerEnemies(float dTime)
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
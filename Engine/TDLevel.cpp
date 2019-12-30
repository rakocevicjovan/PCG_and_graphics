#include "TDLevel.h"
#include "Terrain.h"
#include "Geometry.h"
#include "AStar.h"
#include "Picker.h"
#include "ColFuncs.h"
#include "Shader.h"
#include "Steering.h"

inline float pureDijkstra(const NavNode& n1, const NavNode& n2)
{
	return 0.f;
}



//#define DEBUG_OCTREE



void TDLevel::init(Systems& sys)
{
	skyboxCubeMapper.LoadFromFiles(S_DEVICE, "../Textures/day.dds");
	WAAAT.LoadModel(S_DEVICE, "../Models/ball.fbx");

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
	_navGrid.populate();
	AStar<pureDijkstra>::fillGraph(_navGrid._cells, _navGrid._edges, GOAL_INDEX);
	_navGrid.setGoalIndex(GOAL_INDEX);
	_navGrid.fillFlowField();

	creeps.reserve(NUM_ENEMIES);
	for (int i = 0; i < NUM_ENEMIES; ++i)
	{
		//float offset = (i % 10) * ((i % 2) * 2 - 1);
		SVec3 pos = SVec3(200, 0, 200) + 5 * SVec3(i % 10, 0, (i / 10) % 10);
		//creeps.emplace_back(SMatrix::CreateTranslation(pos), GraphicComponent(resources.getByName<Model*>("FlyingMage"), &randy._shMan.light));
		creeps.emplace_back(SMatrix::CreateTranslation(pos), S_RESMAN.getByName<Model*>("FlyingMage"));
		
		for (Renderable& r : creeps[i].renderables)
		{
			r.mat = S_MATCACHE.getMaterial("creepMat");
			r.pLight = &pLight;
		}

		_octree.insertObject(static_cast<SphereHull*>(creeps[i].collider->hulls.back()));
	}

	tower = Actor(SMatrix(), S_RESMAN.getByName<Model*>("GuardTower"));
	//tower.
	

	/*
	Procedural::Geometry g;
	g.GenBox(SVec3(_navGrid.getCellSize().x, 1, _navGrid.getCellSize().y));
	boxModel.meshes.push_back(Mesh(g, S_DEVICE, true, false));
	box = Actor(SMatrix(), &boxModel);
	box.renderables[0].mat = &creepMat;
	box.renderables[0].pLight = &pLight;
	*/

#ifdef DEBUG_OCTREE
	Procedural::Geometry g;
	g.GenBox(SVec3(1));
	debugModel.meshes.push_back(Mesh(g, S_DEVICE));
	tempBoxes.reserve(1000);
	octNodeMatrices.reserve(1000);
#endif
}



void TDLevel::update(const RenderContext& rc)
{
#ifdef DEBUG_OCTREE
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

	ProcessSpecialInput(rc.dTime);

	//this seems...unnecessarily slow, but i can't rely on pointing back
	for (const Actor& creep : creeps)
	{
		for (auto& hull : creep.collider->hulls)
			hull->setPosition(creep.getPosition());
	}
	

	//this works well to reduce the number of checked branches with simple if(null) but only profiling
	//can tell if it's better this way or by just leaving them allocated (which means deeper checks, but less allocations)
	//Another alternative is having a bool empty; in the octnode...
	_octree.updateAll();	//@TODO probably should optimize this
	_octree.lazyTrim();
	_octree.collideAll();	//@TODO this as well?
	

	//not known to individuals as it depends on group size, therefore should not be in a unit component I'd say... 
	SVec2 stopArea(sqrt(creeps.size()), sqrt(creeps.size()));
	stopArea *= 3.f;
	float stopDistance = stopArea.Length();


	for (int i = 0; i < creeps.size(); ++i)
	{
		//pathfinding and steering, needs to turn off once nobody is moving...

		if (creeps[i]._steerComp._active)
		{
			std::list<Actor*> neighbourCreeps;
			_octree.findWithin(creeps[i].getPosition(), 2.f, neighbourCreeps);
			creeps[i]._steerComp.update(_navGrid, rc.dTime, neighbourCreeps, i, stopDistance);
		}

		//height
		float h = terrain.getHeightAtPosition(creeps[i].getPosition());
		float intervalPassed = fmod(rc.elapsed * 5.f + i * 2.f, 10.f);
		float sway = intervalPassed < 5.f ? Math::smoothstep(0, 5, intervalPassed) : Math::smoothstep(10, 5, intervalPassed);
		Math::setHeight(creeps[i].transform, h + 2 * sway + FLYING_HEIGHT);

		//propagate transforms to children
		creeps[i].propagate();
	}



	//picking, put this away somewhere else...
	if (_sys._inputManager.isKeyDown('R'))
	{
		MCoords mc = _sys._inputManager.getAbsXY();

		Picker p;
		ray = p.generateRay(_sys.getWinW(), _sys.getWinH(), mc.x, mc.y, *rc.cam);

		for (int i = 0; i < creeps.size(); ++i)
		{
			if (Col::RaySphereIntersection(ray, *static_cast<SphereHull*>(creeps[i].collider->hulls[0])))
			{
				Math::RotateMatByQuat(creeps[i].transform, SQuat(SVec3(0, 1, 0), 1.f * rc.dTime));
			}
		}

		SVec3 floorIntersect;
		ray.direction *= 500.f;
		Col::RayPlaneIntersection(ray, SVec3(0, 0, 0), SVec3(1, 0, 0), SVec3(0, 0, 1), floorIntersect);

		creeps[0].transform = SMatrix::CreateTranslation(floorIntersect);
	}



	/// FRUSTUM CULLING
	numCulled = 0;
	const SMatrix v = rc.cam->GetViewMatrix();
	const SVec3 v3c(v._13, v._23, v._33);
	const SVec3 camPos = rc.cam->GetPosition();
	
	
	//cull and add to render queue
	for (int i = 0; i < creeps.size(); ++i)
	{
		if(Col::FrustumSphereIntersection(rc.cam->frustum, *static_cast<SphereHull*>(creeps[i].collider->hulls[0])))
		{
			float zDepth = (creeps[i].transform.Translation() - camPos).Dot(v3c);
			for (auto& r : creeps[i].renderables)
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



void TDLevel::draw(const RenderContext& rc)
{
	rc.d3d->ClearColourDepthBuffers();
	rc.d3d->setRSSolidNoCull();

	S_SHADY.light.SetShaderParameters(S_CONTEXT, floorModel.transform, *rc.cam, pLight, rc.dTime);
	floorModel.Draw(S_CONTEXT, S_SHADY.light);
	S_SHADY.light.ReleaseShaderParameters(S_CONTEXT);

#ifdef DEBUG_OCTREE
	shady.instanced.SetShaderParameters(context, debugModel, *rc.cam, pLight, rc.dTime);
	debugModel.DrawInstanced(context, shady.instanced);
	shady.instanced.ReleaseShaderParameters(context);
#endif

	
	S_RANDY.sortRenderQueue();
	S_RANDY.flushRenderQueue();
	S_RANDY.clearRenderQueue();
	
	S_RANDY.renderSkybox(*rc.cam, *(S_RESMAN.getByName<Model*>("Skysphere")), skyboxCubeMapper);

	/*
	for (int i = 0; i < _navGrid._cells.size(); i++)
	{
		box.renderables[0].worldTransform = SMatrix::CreateTranslation(_navGrid.cellIndexToPos(i));
		randy.render(box.renderables[0]);
	}
	*/

	std::vector<GuiElement> guiElems =
	{
		{"Octree",	std::string("OCT node count " + std::to_string(_octree.getNodeCount()))},
		{"FPS",		std::string("FPS: " + std::to_string(1 / rc.dTime))},
		{"Culling", std::string("Objects culled:" + std::to_string(numCulled))}
	};
	renderGuiElems(guiElems);

	rc.d3d->EndScene();
}



void TDLevel::demolish()
{
	S_INMAN.unregisterController(&_rtsc);
	finished = true;
}



/* old sphere placement, it's here because it's rad
for (int i = 0; i < 125; ++i)
{
	SVec3 pos = SVec3(i % 5, (i / 5) % 5, (i / 25) % 5) * 20.f + SVec3(5.f);
}*/
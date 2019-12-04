#include "TDLevel.h"
#include "Terrain.h"
#include "Geometry.h"
#include "AStar.h"

inline float pureDijkstra(const NavNode& n1, const NavNode& n2)
{
	return 0.f;
}

//#define DEBUG_OCTREE

void TDLevel::init(Systems& sys)
{
	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/day.dds");

	LightData lightData(SVec3(0.1, 0.7, 0.9), .03f, SVec3(0.8, 0.8, 1.0), .2, SVec3(0.3, 0.5, 1.0), 0.7);
	pLight = PointLight(lightData, SVec4(0, 500, 0, 1));

	float tSize = 200;
	Procedural::Terrain t(2, 2, SVec3(tSize));
	t.setOffset(-tSize * .5f, -0.f, -tSize * .5f);
	t.SetUp(device);
	floorModel = Model(t, device);

	//NodeBase* wat = _sg.insert(pLight);		//decide how to handle node ownership here... 
	//_sg.insert(floorModel);

	_oct.init(AABB(SVec3(), SVec3(100)), 3);	//with depth 5 it is reaaallly big... probably not worth it for my game
	_oct.prellocateRootOnly();	//_oct.preallocateTree();	
	
	boiModel.LoadModel(device, "../Models/Skysphere.fbx");

	for (int i = 0; i < 125; ++i)
	{
		SVec3 pos = SVec3(i % 5, (i / 5) % 5, (i / 25) % 5) * 20.f + SVec3(5.f);
		dubois.emplace_back(SMatrix::CreateTranslation(pos), GraphicComponent(&boiModel, &randy._shMan.light));
		dubois[i].collider = new Collider();
		dubois[i].collider->BVT = BVT_SPHERE;
		dubois[i].collider->dynamic = true;
		dubois[i].collider->hulls.push_back(new SphereHull(pos, 1));	//eliminate duplicate pos, redundant and pain to update

		_oct.insertObject(static_cast<SphereHull*>(dubois[i].collider->hulls.back()));
	}

#ifdef DEBUG_OCTREE
	Procedural::Geometry g;
	g.GenBox(SVec3(1));
	debugModel.meshes.push_back(Mesh(g, device));
	tempBoxes.reserve(1000);
	octNodeMatrices.reserve(1000);
#endif

	std::vector<NavNode> nodes;

	nodes.push_back(NavNode("A"));      //0
	nodes.back().edges = { 0, 1 };

	nodes.push_back(NavNode("B1"));     //1
	nodes.back().edges = { 0, 4, 5 };

	nodes.push_back(NavNode("B2"));     //2
	nodes.back().edges = { 1, 2, 3 };

	nodes.push_back(NavNode("C"));      //3
	nodes.back().edges = { 2 };

	nodes.push_back(NavNode("D1"));     //4
	nodes.back().edges = { 5, 7 };

	nodes.push_back(NavNode("D2"));     //5
	nodes.back().edges = { 3, 4, 6 };

	nodes.push_back(NavNode("END"));    //6
	nodes.back().edges = { 6, 7 };

	std::vector<NavEdge> edges;

	edges.push_back(NavEdge(0, 1)); //A B1
	edges.back().weight = 1;

	edges.push_back(NavEdge(0, 2)); //A B2
	edges.back().weight = 1;

	edges.push_back(NavEdge(2, 3)); //B2 C
	edges.back().weight = 1;

	edges.push_back(NavEdge(2, 5)); //B2 D2
	edges.back().weight = 1;

	edges.push_back(NavEdge(1, 5)); //B1 D2
	edges.back().weight = 1;

	edges.push_back(NavEdge(1, 4)); //B1 D1
	edges.back().weight = 1;

	edges.push_back(NavEdge(5, 6)); //D2 END
	edges.back().weight = 1;

	edges.push_back(NavEdge(4, 6, 20)); //D1 END

	AStar<pureDijkstra>::fillGraph(nodes, edges, 6);

	for (int i = 0; i < nodes.size(); ++i)
	{
		std::string output = "Distance from " + nodes[i].name + " to END node:" + std::to_string(nodes[i].pathWeight) + "\n";
		OutputDebugStringA(output.c_str());
	}
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
	updateCam(rc.dTime);

	for (auto& boi : dubois)
	{
		for (auto& hull : boi.collider->hulls)
			hull->setPosition(boi.getPosition());
	}

	_oct.updateAll();
	_oct.lazyTrim();
	//this works well to reduce the number of checked branches with simple if(null) but only profiling
	//can tell if it's better this way or by just leaving them allocated (which means deeper checks, but less allocations)
	//Another alternative is having a bool empty; in the octnode...
}



void TDLevel::draw(const RenderContext& rc)
{
	rc.d3d->ClearColourDepthBuffers();
	rc.d3d->TurnOffCulling();

	shady.light.SetShaderParameters(context, floorModel.transform, *rc.cam, pLight, rc.dTime);
	floorModel.Draw(context, shady.light);
	shady.light.ReleaseShaderParameters(context);

	auto www = _sys._resMan.getResourceByName("FlyingMage");
	Model* m = static_cast<Model*> (www);

	SMatrix rotMatrix = SMatrix::CreateFromAxisAngle(SVec3(0, 1, 0), .1 * rc.dTime);

	shady.light.SetShaderParameters(context, m->transform, *rc.cam, pLight, rc.dTime);
	m->Draw(context, shady.light);
	shady.light.ReleaseShaderParameters(context);

	randy.RenderSkybox(*rc.cam, skybox, skyboxCubeMapper);

	for (int i = 0; i < dubois.size(); ++i)
	{
		dubois[i].transform = dubois[i].transform * rotMatrix;
		dubois[i].Draw(context, *rc.cam, pLight, rc.dTime);
	}
	
#ifdef DEBUG_OCTREE
	shady.instanced.SetShaderParameters(context, debugModel, *rc.cam, pLight, rc.dTime);
	debugModel.DrawInstanced(context, shady.instanced);
	shady.instanced.ReleaseShaderParameters(context);
#endif

	//renderString("FPS", std::string("FPS: " + std::to_string(1 / rc.dTime)));
	renderString("Octree", std::string("OCT node count " + std::to_string(_oct.getNodeCount())));

	rc.d3d->EndScene();
}
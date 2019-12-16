#include "TDLevel.h"
#include "Terrain.h"
#include "Geometry.h"
#include "AStar.h"
#include "Picker.h"
#include "ColFuncs.h"
#include "Shader.h"

inline float pureDijkstra(const NavNode& n1, const NavNode& n2)
{
	return 0.f;
}

//#define DEBUG_OCTREE

void TDLevel::init(Systems& sys)
{
	skyboxCubeMapper.LoadFromFiles(S_DEVICE, "../Textures/day.dds");

	LightData lightData(SVec3(0.1, 0.7, 0.9), .03f, SVec3(0.8, 0.8, 1.0), .2, SVec3(0.3, 0.5, 1.0), 0.7);
	pLight = PointLight(lightData, SVec4(0, 500, 0, 1));

	float tSize = 400;
	Procedural::Terrain t(2, 2, SVec3(tSize));
	t.setOffset(-tSize * .5f, -0.f, -tSize * .5f);
	t.SetUp(S_DEVICE);
	floorModel = Model(t, S_DEVICE);

	_oct.init(AABB(SVec3(), SVec3(200)), 3);	//with depth 5 it is reaaallly big... probably not worth it for my game
	_oct.prellocateRootOnly();					//_oct.preallocateTree();	




	//@TODO MOVE OUTTA HERE REEEE
	D3D11_BUFFER_DESC matrixBufferDesc, lightBufferDesc;
	matrixBufferDesc = ShaderCompiler::createCBufferDesc(sizeof(SMatrix));
	lightBufferDesc = ShaderCompiler::createCBufferDesc(sizeof(LightBuffer));

	std::vector<D3D11_INPUT_ELEMENT_DESC> inLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D11_SAMPLER_DESC sbSamplerDesc;
	ZeroMemory(&sbSamplerDesc, sizeof(sbSamplerDesc));
	sbSamplerDesc = { D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP,
		0.0f, 1, D3D11_COMPARISON_ALWAYS, 0, 0, 0, 0, 0, D3D11_FLOAT32_MAX };

	VertexShader* vs = new VertexShader();
	PixelShader* ps = new PixelShader();

	_sys._shaderCompiler.compileVS(L"lightvs.hlsl", inLayout, vs->_vShader, vs->_layout);
	vs->_cbuffers.resize(1);
	_sys._shaderCompiler.createConstantBuffer(matrixBufferDesc, vs->_cbuffers[0]);

	_sys._shaderCompiler.compilePS(L"lightps.hlsl", ps->_pShader);
	ps->_cbuffers.resize(1);
	_sys._shaderCompiler.createConstantBuffer(lightBufferDesc, ps->_cbuffers[0]);

	_sys._shaderCompiler.createSamplerState(sbSamplerDesc, ps->_sState);

	creepMat.opaque = true;
	creepMat.setVS(vs);
	creepMat.setPS(ps);
	creepMat.textures.push_back(&(resources.getByName<Model*>("FlyingMage")->meshes[0].textures[0]));

	creeps.reserve(125);
	for (int i = 0; i < 125; ++i)
	{
		SVec3 pos = SVec3(200, 0, 200);
		//creeps.emplace_back(SMatrix::CreateTranslation(pos), GraphicComponent(resources.getByName<Model*>("FlyingMage"), &randy._shMan.light));
		creeps.emplace_back(SMatrix::CreateTranslation(pos), resources.getByName<Model*>("FlyingMage"));
		
		for (Renderable& r : creeps[i].renderables)
		{
			r.mat = &creepMat;
			r.pLight = &pLight;
		}
		
		creeps[i].collider = new Collider();
		creeps[i].collider->actParent = &creeps.back();
		creeps[i].collider->BVT = BVT_SPHERE;
		creeps[i].collider->dynamic = true;
		creeps[i].collider->hulls.push_back(new SphereHull(pos, 1));	//eliminate duplicate pos, redundant and pain to update
		creeps[i].collider->hulls.back()->_collider = creeps[i].collider;

		_oct.insertObject(static_cast<SphereHull*>(creeps[i].collider->hulls.back()));
	}

#ifdef DEBUG_OCTREE
	Procedural::Geometry g;
	g.GenBox(SVec3(1));
	debugModel.meshes.push_back(Mesh(g, device));
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
	updateCam(rc.dTime);

	//this seems...unnecessarily slow, but i can't rely on pointing back
	for (const Actor& creep : creeps)
	{
		for (auto& hull : creep.collider->hulls)
			hull->setPosition(creep.getPosition());
	}
	

	//this works well to reduce the number of checked branches with simple if(null) but only profiling
	//can tell if it's better this way or by just leaving them allocated (which means deeper checks, but less allocations)
	//Another alternative is having a bool empty; in the octnode...
	_oct.updateAll();	//@TODO probably should optimize this
	_oct.lazyTrim();
	_oct.collideAll();	//@TODO this as well?
	


	for (Actor& act : creeps)
	{
		for (Renderable& r : act.renderables)
		{
			r.worldTransform = act.transform * r.transform;
		}
	}

	//picking, put this away somewhere else...
	if (_sys._inputManager.isKeyDown('R'))
	{
		MCoords mc = _sys._inputManager.getAbsXY();

		Picker p;
		ray = p.generateRay(_sys.getWinW(), _sys.getWinH(), mc.x, mc.y, *rc.cam);

		for (int i = 0; i < creeps.size(); ++i)
		{
			float t;
			SVec3 ip;
			if (Col::RaySphereIntersection(ray, *static_cast<SphereHull*>(creeps[i].collider->hulls[0])))
			{
				Math::RotateMatByQuat(creeps[i].transform, SQuat(SVec3(0, 1, 0), 1.f * rc.dTime));
			}
		}
	}



	/// FRUSTUM CULLING
	numCulled = 0;
	const SMatrix v = rc.cam->GetViewMatrix();
	const SVec3 v3c(v._13, v._23, v._33);
	const SVec3 camPos = rc.cam->GetPosition();

	for (int i = 0; i < creeps.size(); ++i)
	{
		if(Col::FrustumSphereIntersection(rc.cam->frustum, *static_cast<SphereHull*>(creeps[i].collider->hulls[0])))
		{
			//add to draw queue and all that jazz
			float zDepth = (creeps[i].transform.Translation() - camPos).Dot(v3c);
			for (auto& r : creeps[i].renderables)
			{
				r.zDepth = zDepth;
				randy.addToRenderQueue(r);
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

	shady.light.SetShaderParameters(context, floorModel.transform, *rc.cam, pLight, rc.dTime);
	floorModel.Draw(context, shady.light);
	shady.light.ReleaseShaderParameters(context);

	randy.RenderSkybox(*rc.cam, *(resources.getByName<Model*>("Skysphere")), skyboxCubeMapper);

#ifdef DEBUG_OCTREE
	shady.instanced.SetShaderParameters(context, debugModel, *rc.cam, pLight, rc.dTime);
	debugModel.DrawInstanced(context, shady.instanced);
	shady.instanced.ReleaseShaderParameters(context);
#endif

	std::vector<GuiElement> guiElems = 
	{ 
		{"Octree", std::string("OCT node count " + std::to_string(_oct.getNodeCount()))},
		{"FPS", std::string("FPS: " + std::to_string(1 / rc.dTime))},
		{"Culling", std::string("Objects culled:" + std::to_string(numCulled))}
	};
	renderGuiElems(guiElems);
	
	randy.sortRenderQueue();
	randy.flushRenderQueue();
	randy.clearRenderQueue();

	rc.d3d->EndScene();
}



/* old sphere placement, I quite like it
for (int i = 0; i < 125; ++i)
{
	SVec3 pos = SVec3(i % 5, (i / 5) % 5, (i / 25) % 5) * 20.f + SVec3(5.f);
}*/
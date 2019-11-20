#include "TDLevel.h"
#include "Terrain.h"

void TDLevel::init(Systems& sys)
{
	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/day.dds");

	LightData lightData(SVec3(0.1, 0.7, 0.9), .03f, SVec3(0.8, 0.8, 1.0), .2, SVec3(0.3, 0.5, 1.0), 0.7);
	pLight = PointLight(lightData, SVec4(0, 500, 0, 1));

	float tSize = 200;
	Procedural::Terrain t(2, 2, SVec3(tSize));
	t.setOffset(-tSize * .5f, -10.f, -tSize * .5f);
	t.SetUp(device);
	floorModel = Model(t, device);

	NodeBase* wat = _sg.insert(pLight);		//decide how to handle node ownership here... 
	_sg.insert(floorModel);

	_oct.init(AABB(SVec3(), SVec3(100)), 5);
	_oct.prellocateRootOnly();
	//_oct.preallocateTree();

	boiModel.LoadModel(device, "../Models/Skysphere.fbx");

	Actor a();

	for (int i = 0; i < 125; ++i)
	{
		SVec3 pos = SVec3(i % 5, (i / 5) % 5, (i / 25) % 5) * 20.f;
		dubois.emplace_back(SMatrix::CreateTranslation(pos), GraphicComponent(&boiModel, &randy._shMan.light));
		dubois[i].collider = new Collider();
		dubois[i].collider->BVT = BVT_SPHERE;
		dubois[i].collider->dynamic = true;
		dubois[i].collider->hulls.push_back(new SphereHull(pos, 1));	//eliminate duplicate pos, redundant and pain to update

		_oct.insertObject(_oct._rootNode, static_cast<SphereHull*>(dubois[i].collider->hulls.back()));
	}

	std::vector<AABB> tempBoxes;
	tempBoxes.reserve(100);
	_oct.getTreeAsAABBVector(tempBoxes);

	for (int i = 0; i < tempBoxes.size(); ++i)
	{
		debugModel.meshes.push_back(Mesh(static_cast<Hull*>(&tempBoxes[i]), device));
	}

}



void TDLevel::update(const RenderContext& rc)
{
	ProcessSpecialInput(rc.dTime);
	updateCam(rc.dTime);
}



void TDLevel::draw(const RenderContext& rc)
{
	rc.d3d->ClearColourDepthBuffers();
	rc.d3d->TurnOffCulling();

	shady.light.SetShaderParameters(context, floorModel.transform, *rc.cam, pLight, rc.dTime);
	floorModel.Draw(context, shady.light);
	shady.light.ReleaseShaderParameters(context);

	randy.RenderSkybox(*rc.cam, skybox, skyboxCubeMapper);

	for (int i = 0; i < dubois.size(); ++i)
	{
		dubois[i].Draw(context, *rc.cam, pLight, rc.dTime);
	}

	shady.wireframe.SetShaderParameters(context, debugModel, rc.cam->GetViewMatrix(), rc.cam->GetProjectionMatrix());
	debugModel.Draw(context, shady.wireframe);
	shady.wireframe.ReleaseShaderParameters(context);

	rc.d3d->EndScene();
}
#include "AirLevel.h"



void AirLevel::init(Systems& sys)
{
	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/day.dds");

	/*Procedural::Terrain barrensTerrain;
	auto fltVec = Texture::generateRidgey(256, 256, 0.f, 1.61803f, 0.5793f, 1.f, 6u);
	barrensTerrain.setScales(2, 128, 2);
	barrensTerrain.GenFromTexture(256, 256, fltVec);
	barrensTerrain.setOffset(-256, 0, -256);
	barrensTerrain.setTextureData(device, 10, 10, { "../Textures/LavaCracks/diffuse.png", "../Textures/LavaCracks/normal.png" });
	barrensTerrain.SetUp(device);
	
	barrens = Model(barrensTerrain, device);
	*/

	LightData lightData(SVec3(0.1, 0.7, 0.9), .03f, SVec3(0.8, 0.8, 1.0), .2, SVec3(0.3, 0.5, 1.0), 0.7);
	pointLight = PointLight(lightData, SVec4(0, 500, 0, 1));
	dirLight = DirectionalLight(lightData, SVec4(0, -1, 0, 1));
	lightView = SMatrix::CreateLookAt(SVec3(pointLight.pos), SVec3(), SVec3(0, 0, 1));
	headModel.LoadModel(device, "../Models/Ball.fbx");	//../Models/Dragon/dragonhead.obj
	segmentModel.LoadModel(device, "../Models/Ball.fbx");

	dragon.init(headModel, segmentModel);
}



void AirLevel::draw(const RenderContext& rc)
{
	rc.d3d->ClearColourDepthBuffers(rc.d3d->clearColour);
	ProcessSpecialInput(rc.dTime);
	updateCam(rc.dTime);

	dragon.update(rc, windDir * windInt);

	shady.strife.SetShaderParameters(context, headModel, *rc.cam, dirLight, rc.elapsed, nullptr, nullptr, SMatrix());
	headModel.Draw(context, shady.strife);
	shady.strife.ReleaseShaderParameters(context);


	//shady.terrainNormals.SetShaderParameters(context, barrens.transform, *rc.cam, pointLight, rc.dTime);
	//barrens.Draw(context, shady.terrainNormals);

	randy.RenderSkybox(*rc.cam, skybox, skyboxCubeMapper);

	rc.d3d->EndScene();
}

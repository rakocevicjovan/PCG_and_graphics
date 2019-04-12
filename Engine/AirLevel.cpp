#include "AirLevel.h"



void AirLevel::init(Systems& sys)
{
	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/day.dds");

	Procedural::Terrain barrensTerrain;
	auto fltVec = Texture::generateRidgey(256, 256, 0.f, 1.61803f, 0.5793f, 1.f, 6u);
	barrensTerrain.setScales(2, 128, 2);
	barrensTerrain.GenFromTexture(256, 256, fltVec);
	barrensTerrain.setOffset(-256, 0, -256);
	barrensTerrain.setTextureData(device, 10, 10, { "../Textures/LavaCracks/diffuse.png", "../Textures/LavaCracks/normal.png" });
	barrensTerrain.SetUp(device);

	barrens = Model(barrensTerrain, device);

	LightData lightData(SVec3(0.1f, 0.7f, 0.9f), .03f, SVec3(0.8f, 0.8f, 1.0f), .2f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);
	pointLight = PointLight(lightData, SVec4(0, 500.f, 0, 1.0f));
	
}



void AirLevel::draw(const RenderContext& rc)
{
	rc.d3d->ClearColourDepthBuffers(rc.d3d->clearColour);
	ProcessSpecialInput(rc.dTime);

	updateCam(rc.dTime);

	_sys._renderer._shMan.terrainNormals.SetShaderParameters(context, barrens.transform, *rc.cam, pointLight, rc.dTime);
	barrens.Draw(context, shady.terrainNormals);

	randy.RenderSkybox(*rc.cam, skybox, skyboxCubeMapper);

	rc.d3d->EndScene();
}

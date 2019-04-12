#include "AirLevel.h"



void AirLevel::init(Systems& sys)
{
	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/day.dds");

	Procedural::Terrain barrensTerrain;
	
	barrens = Model(barrensTerrain, device);

	barrens.meshes.emplace_back(barrensTerrain, sys._device);

	
}



void AirLevel::draw(const RenderContext& rc)
{
	rc.d3d->ClearColourDepthBuffers(rc.d3d->clearColour);
	ProcessSpecialInput(rc.dTime);

	updateCam(rc.dTime);

	_sys._renderer.RenderSkybox(*rc.cam, skybox, skyboxCubeMapper);

	rc.d3d->EndScene();
}

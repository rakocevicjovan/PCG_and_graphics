#include "EarthLevel.h"


void EarthLevel::init(Systems& sys)
{
	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/night.dds");

	will.LoadModel(device, "../Models/ball.fbx");
	Math::Scale(will.transform, SVec3(5.f));
	Math::Translate(will.transform, SVec3(2, 35, 60));

	LightData lightData(SVec3(0.1f, 0.7f, 0.9f), .03f, SVec3(0.8f, 0.8f, 1.0f), .2f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);
	pointLight = PointLight(lightData, SVec4(333.f, 666.f, 999.f, 1.0f));	//old moon position SVec4(50.0f, 250.f, 250.0f, 1.0f)

	proceduralTerrain = Procedural::Terrain(320, 320);
	proceduralTerrain.setTextureData(device, 16, 16, { "../Textures/Lava/diffuse.jpg", "../Textures/Lava/normal.jpg" });
	proceduralTerrain.SetUp(device);

	mazeDiffuseMap = Texture(device, "../Textures/Rock/diffuse.jpg");
	mazeNormalMap = Texture(device, "../Textures/Rock/normal.jpg");
	mazeDisplacementMap = Texture(device, "../Textures/Rock/height.png");

	maze.Init(10, 10, 32.f);
	maze.CreateModel(device);

	sys._colEngine.registerModel(maze.model, BoundingVolumeType::BVT_AABB);
}




void EarthLevel::procGen()
{

}



void EarthLevel::draw(const RenderContext& rc)
{
	updateCam(rc.dTime);

	_sys._deviceContext->RSSetViewports(1, &rc.d3d->viewport);
	rc.d3d->SetBackBufferRenderTarget();
	rc.d3d->ClearColourDepthBuffers(rc.d3d->clearColour);

	proceduralTerrain.Draw(context, rc.shMan->terrainNormals, *rc.cam, pointLight, rc.elapsed);

	rc.shMan->dynamicHeightMaze.SetShaderParameters(context, maze.model, *rc.cam, pointLight, rc.elapsed, mazeDiffuseMap, mazeNormalMap, mazeDisplacementMap);
	maze.model.Draw(context, rc.shMan->dynamicHeightMaze);
	rc.shMan->dynamicHeightMaze.ReleaseShaderParameters(context);

	randy.RenderSkybox(*rc.cam, skybox, skyboxCubeMapper);

	rc.d3d->TurnOnAlphaBlending();
	rc.shMan->shVolumAir.SetShaderParameters(context, will, *rc.cam, rc.elapsed);
	will.Draw(context, rc.shMan->shVolumAir);
	rc.d3d->TurnOffAlphaBlending();

	//move out of here
	if (!_sys._controller.isFlying())
	{
		SVec3 oldPos = _sys._renderer._cam.GetCameraMatrix().Translation();
		float newHeight = proceduralTerrain.getHeightAtPosition(_sys._renderer._cam.GetCameraMatrix().Translation());
		rc.cam->SetTranslation(SVec3(oldPos.x, newHeight, oldPos.z));
	}

	rc.d3d->EndScene();

	ProcessSpecialInput(rc.dTime);
}
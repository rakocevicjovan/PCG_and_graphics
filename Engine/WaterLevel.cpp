#include "WaterLevel.h"



void WaterLevel::init(Systems& sys)
{
	_sys._colEngine.registerController(_sys._controller);

	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/night.dds");

	fence.LoadModel(device, "../Models/WaterFence.obj");

	cubeMapper.Init(device);

	will.LoadModel(device, "../Models/ball.fbx");

	modBall.LoadModel(device, "../Models/ball.fbx");
	Math::Scale(modBall.transform, SVec3(10));
	Math::Translate(modBall.transform, SVec3(0, 164, 0));

	lotus.LoadModel(device, "../Models/crownless.obj");
	Math::Scale(lotus.transform, SVec3(100));

	lotusTex.LoadFromFile("../Textures/Lotus/diffuse.jpg");
	lotusTex.Setup(device);
	for (auto& m : lotus.meshes)
		m.textures.push_back(lotusTex);

	waterTerrain = Procedural::Terrain(256, 256, SVec3(2, 1, 2));
	waterTerrain.setTextureData(device, 10, 10, { "../Textures/LavaIntense/diffuse.jpg", "../Textures/LavaIntense/normal.jpg" });
	waterTerrain.setOffset(-256, 96, -256);
	waterTerrain.CalculateTexCoords();
	waterTerrain.CalculateNormals();

	waterSheet = Model(waterTerrain, _sys._device);

	waterNoiseMap.LoadFromFile("../Textures/Noiz.png");
	waterNoiseMap.Setup(device);
	
	reflectionMap.Init(device, 1600, 900);	//@make false after testing for both of them!
	refractionMap.Init(device, 1600, 900);
	clipper = SPlane(SVec3(0, 1, 0), -32);

	LightData lightData(SVec3(0.1f, 0.7f, 0.9f), .03f, SVec3(0.8f, 0.8f, 1.0f), .2f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);
	pointLight = PointLight(lightData, SVec4(0, 500.f, 0, 1.0f));
}


void WaterLevel::draw(const RenderContext& rc)
{
	updateCam(rc.dTime);

	rc.d3d->BeginScene(rc.d3d->clearColour);

	dc->RSSetViewports(1, &(cubeMapper.cm_viewport));
	cubeMapper.UpdateCams(modBall.transform.Translation());

	for (int i = 0; i < 6; i++)
	{
		cubeMapper.Advance(_sys._deviceContext, i);

		//lotus
		rc.shMan->light.SetShaderParameters(dc, lotus, cubeMapper.getCameraAtIndex(i), pointLight, rc.dTime);
		lotus.Draw(dc, rc.shMan->light);
		rc.shMan->light.ReleaseShaderParameters(dc);

		//water
		rc.shMan->water.SetShaderParameters(dc, waterSheet, cubeMapper.getCameraAtIndex(i), pointLight, rc.elapsed, waterNoiseMap.srv, waterNoiseMap.srv, waterNoiseMap.srv);
		waterSheet.Draw(dc, rc.shMan->water);
		rc.shMan->water.ReleaseShaderParameters(dc);

		_sys._renderer.RenderSkybox(cubeMapper.getCameraAtIndex(i), skybox, cubeMapper, skyboxCubeMapper);
	}

	dc->RSSetViewports(1, &rc.d3d->viewport);
	rc.d3d->SetBackBufferRenderTarget();

	//lotus
	rc.shMan->light.SetShaderParameters(dc, lotus, *rc.cam, pointLight, rc.dTime);
	lotus.Draw(dc, rc.shMan->light);
	rc.shMan->light.ReleaseShaderParameters(dc);

	//water
	rc.shMan->water.SetShaderParameters(dc, waterSheet, *rc.cam, pointLight, rc.elapsed, waterNoiseMap.srv, waterNoiseMap.srv, waterNoiseMap.srv);
	waterSheet.Draw(dc, rc.shMan->water);
	rc.shMan->water.ReleaseShaderParameters(dc);

	//reflection sphere
	rc.shMan->cubeMapShader.SetShaderParameters(dc, modBall, *rc.cam, pointLight, rc.dTime, cubeMapper.cm_srv);
	modBall.Draw(dc, rc.shMan->cubeMapShader);
	rc.shMan->cubeMapShader.ReleaseShaderParameters(dc);

	//SKYBOX
	rc.d3d->TurnOffCulling();
	rc.d3d->SwitchDepthToLessEquals();
	rc.shMan->skyboxShader.SetShaderParameters(dc, skybox.transform, *rc.cam, rc.dTime, skyboxCubeMapper.cm_srv);
	skybox.Draw(dc, rc.shMan->skyboxShader);
	rc.shMan->skyboxShader.ReleaseShaderParameters(dc);
	rc.d3d->SwitchDepthToDefault();
	rc.d3d->TurnOnCulling();

	/*
	rc.d3d->TurnOnAlphaBlending();
	rc.shMan->shVolumAir.SetShaderParameters(dc, will, *rc.cam, rc.elapsed);
	will.Draw(dc, rc.shMan->shVolumAir);
	rc.d3d->TurnOffAlphaBlending();
	*/

	rc.d3d->EndScene();
	ProcessSpecialInput(rc.dTime);
}
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
	Math::Translate(modBall.transform, SVec3(0, 128, 0));

	lotus.LoadModel(device, "../Models/crownless.obj");
	Math::Scale(lotus.transform, SVec3(100));

	lotusTex.LoadFromFile("../Textures/Lotus/diffuse.jpg");
	lotusTex.Setup(device);
	for (auto& m : lotus.meshes)
		m.textures.push_back(lotusTex);

	waterTerrain = Procedural::Terrain(256, 256);
	waterTerrain.setTextureData(device, 10, 10, { "../Textures/LavaIntense/diffuse.jpg", "../Textures/LavaIntense/normal.jpg" });
	waterTerrain.setOffset(0, 32, 0);
	waterTerrain.CalculateTexCoords();
	waterTerrain.CalculateNormals();

	waterSheet = Model(waterTerrain, _sys._device);

	waterNoiseMap.LoadFromFile("../Textures/Noiz.png");
	waterNoiseMap.Setup(device);
	
	reflectionMap.Init(device, 1600, 900);	//@make false after testing for both of them!
	refractionMap.Init(device, 1600, 900);
	clippingPlane = SPlane(SVec3(0, 1, 0), 32);

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
		/////////////		@TODO MOVE THESE TWO TO THE BOTTOM OF THE LOOP AFTER OTHER STUFF IS DONE
		dc->ClearRenderTargetView(cubeMapper.cm_rtv[i], cubeMapper.clearCol);
		dc->ClearDepthStencilView(cubeMapper.cm_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		/////////////
		dc->OMSetRenderTargets(1, &cubeMapper.cm_rtv[i], cubeMapper.cm_depthStencilView);

		rc.d3d->TurnOffCulling();
		rc.d3d->SwitchDepthToLessEquals();
		rc.shMan->shaderSkybox.SetShaderParameters(dc, skybox.transform, cubeMapper.getCameraAtIndex(i), rc.dTime, skyboxCubeMapper.cm_srv);
		skybox.Draw(dc, rc.shMan->shaderSkybox);
		rc.shMan->shaderSkybox.ReleaseShaderParameters(dc);
		rc.d3d->SwitchDepthToDefault();
		rc.d3d->TurnOnCulling();
	}

	dc->RSSetViewports(1, &rc.d3d->viewport);
	rc.d3d->SetBackBufferRenderTarget();

	//lotus
	rc.shMan->shaderLight.SetShaderParameters(dc, lotus, *rc.cam, pointLight, rc.dTime);
	lotus.Draw(dc, rc.shMan->shaderLight);
	rc.shMan->shaderLight.ReleaseShaderParameters(dc);

	//water
	rc.shMan->shaderWater.SetShaderParameters(dc, waterSheet, *rc.cam, pointLight, rc.elapsed, waterNoiseMap.srv, waterNoiseMap.srv, waterNoiseMap.srv);
	waterSheet.Draw(dc, rc.shMan->shaderWater);
	rc.shMan->shaderWater.ReleaseShaderParameters(dc);

	//reflection sphere
	rc.shMan->shaderCM.SetShaderParameters(dc, modBall, *rc.cam, pointLight, rc.dTime, cubeMapper.cm_srv);
	modBall.Draw(dc, rc.shMan->shaderCM);
	rc.shMan->shaderCM.ReleaseShaderParameters(dc);

	//SKYBOX
	rc.d3d->TurnOffCulling();
	rc.d3d->SwitchDepthToLessEquals();
	rc.shMan->shaderSkybox.SetShaderParameters(dc, skybox.transform, *rc.cam, rc.dTime, skyboxCubeMapper.cm_srv);
	skybox.Draw(dc, rc.shMan->shaderSkybox);
	rc.shMan->shaderSkybox.ReleaseShaderParameters(dc);
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
#include "WaterLevel.h"



void WaterLevel::init(Systems& sys)
{
	skybox.LoadModel(device, "../Models/Skysphere.fbx");
	skyboxCubeMapper.LoadFromFiles(device, "../Textures/night.dds");

	cubeMapper.Init(device);

	will.LoadModel(device, "../Models/ball.fbx");
	Math::Scale(will.transform, SVec3(5.f));
	Math::Translate(will.transform, SVec3(2, 35, 60));
	modBall.LoadModel(device, "../Models/ball.fbx");

	lotus.LoadModel(device, "../Models/crownles_smoother.obj");
	Math::Scale(lotus.transform, SVec3(100));

	lotusTex.LoadFromFile("../Textures/Lotus/diffuse.jpg");
	lotusTex.Setup(device);
	for (auto& m : lotus.meshes)
		m.textures.push_back(lotusTex);


	LightData lightData(SVec3(0.1f, 0.7f, 0.9f), .03f, SVec3(0.8f, 0.8f, 1.0f), .2f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);
	pointLight = PointLight(lightData, SVec4(0, 500.f, 0, 1.0f));
}



void WaterLevel::draw(const RenderContext& rc)
{
	rc.d3d->BeginScene(rc.d3d->clearColour);

	dc->RSSetViewports(1, &(cubeMapper.cm_viewport));
	cubeMapper.UpdateCams(modBall.transform.Translation());

	for (int i = 0; i < 6; i++)
	{
		dc->ClearRenderTargetView(cubeMapper.cm_rtv[i], cubeMapper.clearCol);
		dc->ClearDepthStencilView(cubeMapper.cm_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
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

	//REFLECTION SPHERE
	rc.shMan->shaderCM.SetShaderParameters(dc, modBall, *rc.cam, pointLight, rc.dTime, cubeMapper.cm_srv);
	modBall.Draw(dc, rc.shMan->shaderCM);
	rc.shMan->shaderCM.ReleaseShaderParameters(dc);


	rc.shMan->shaderLight.SetShaderParameters(dc, lotus, *rc.cam, pointLight, rc.dTime);
	lotus.Draw(dc, rc.shMan->shaderLight);
	rc.shMan->shaderLight.ReleaseShaderParameters(dc);

	//SKYBOX
	rc.d3d->TurnOffCulling();
	rc.d3d->SwitchDepthToLessEquals();
	rc.shMan->shaderSkybox.SetShaderParameters(dc, skybox.transform, *rc.cam, rc.dTime, skyboxCubeMapper.cm_srv);
	skybox.Draw(dc, rc.shMan->shaderSkybox);
	rc.shMan->shaderSkybox.ReleaseShaderParameters(dc);
	rc.d3d->SwitchDepthToDefault();
	rc.d3d->TurnOnCulling();


	rc.d3d->TurnOnAlphaBlending();
	rc.shMan->shVolumAir.SetShaderParameters(dc, will, *rc.cam, rc.elapsed);
	will.Draw(dc, rc.shMan->shVolumAir);
	rc.d3d->TurnOffAlphaBlending();

	rc.d3d->EndScene();
	ProcessSpecialInput(rc.dTime);
}
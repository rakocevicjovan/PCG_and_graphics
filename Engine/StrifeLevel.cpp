#include "StrifeLevel.h"

namespace Strife
{

	void StrifeLevel::init(Systems& sys)
	{
		skyboxCubeMapper.LoadFromFiles(device, "../Textures/night.dds");

		skybox.LoadModel(device, "../Models/Skysphere.fbx");
		//cloudscape = skybox;

		LightData lightData(SVec3(0.1f, 0.7f, 0.9f), 32000.f, SVec3(0.8f, 0.8f, 1.0f), .2f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);

		float edge = 256;
		terrain = Procedural::Terrain(2, 2, SVec3(edge, 1, edge));
		terrain.setOffset(-edge * .5, -32, -edge * .5);
		terrain.SetUp(device);

		sky = Procedural::Terrain(2, 2, SVec3(edge * 4, 1, edge * 4));
		sky.CalculateNormals();
		cloudscape = Model(sky, device);
		cloudscape.transform = SMatrix::CreateRotationZ(PI);
		Math::SetTranslation(cloudscape.transform, SVec3(edge * 2, 400, -edge * 2));

		sys._D3D.SetBackBufferRenderTarget();

		csDef.celestial = PointLight(lightData, SVec4(333.f, 666.f, 999.f, 1.0f));	//old moon position SVec4(50.0f, 250.f, 250.0f, 1.0f)
		csDef.coverage_broad = Texture(device, "../Textures/worley.png");
		csDef.coverage_frequent = Texture(device, "../Textures/highDetail.jpg");
		csDef.heightMask = SVec2(400, 600);
		csDef.planeMat = cloudscape.transform;
	}



	void StrifeLevel::procGen()
	{

	}



	void StrifeLevel::update(const RenderContext& rc)
	{
		if (_sys._inputManager.IsKeyDown((short)'M') && sinceLastInput > .33f)
		{
			inman.ToggleMouseMode();
			sinceLastInput = 0;
		}

		if(!inman.GetMouseMode())
			updateCam(rc.dTime);

		sinceLastInput += rc.dTime;

		csDef.planeMat.m[3][1] = csDef.heightMask.x;
		//Math::SetTranslation(csDef.planeMat, SVec3(0, csDef.heightMask.x, 0));
	}



	void StrifeLevel::draw(const RenderContext& rc)
	{
		rc.d3d->ClearColourDepthBuffers();

		terrain.Draw(context, shady.light, *rc.cam, csDef.celestial, rc.dTime);

		randy.RenderSkybox(*rc.cam, skybox, skyboxCubeMapper);

		rc.d3d->TurnOnAlphaBlending();

		shady.strife.SetShaderParameters(context, *rc.cam, csDef, rc.elapsed);

		cloudscape.Draw(context, shady.strife);

		shady.strife.ReleaseShaderParameters(context);

		rc.d3d->TurnOffAlphaBlending();

		if(inman.GetMouseMode())
			ToolGUI::Render(csDef);

		rc.d3d->EndScene();
	}

}
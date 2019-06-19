#include "StrifeLevel.h"
#include "ShaderUtils.h"

namespace Strife
{

	void StrifeLevel::init(Systems& sys)
	{
		skyboxCubeMapper.LoadFromFiles(device, "../Textures/night.dds");

		skybox.LoadModel(device, "../Models/Skysphere.fbx");

		//calculate dimensions of screen quad so that it fits the screen - unless d
		Mesh scrQuadMesh = Mesh(SVec2(0., 0.), SVec2(1.f, 1.f), device, .999999f);	//1.777777f
		screenQuad.meshes.push_back(scrQuadMesh);

		_sys._renderer._cam.SetProjectionMatrix(DirectX::XMMatrixPerspectiveFovLH(0.5 * PI, randy._screenAspect, 1.f, 1000.f));

		//32000.f in lux but that doesn't work... not sure how to do any of this
		LightData lightData(SVec3(1.f, 1.f, 1.f), 1.f, SVec3(0.8f, 0.8f, 1.0f), .2f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);

		float edge = 256;
		Procedural::Terrain terrain(2, 2, SVec3(edge, 1, edge));
		terrain.setOffset(-edge * .5, 0.f, -edge * .5);
		terrain.CalculateNormals();

		floor = Model(terrain, device);
		floor.transform = SMatrix::CreateTranslation(terrain.getOffset());

		sys._D3D.SetBackBufferRenderTarget();

		csDef.celestial = PointLight(lightData, SVec4(0., 999., 999., 1.0f));
		csDef.rgb_sig_absorption = SVec3(0.5, 1., 2.);
		csDef.eccentricity = 0.8f;
		csDef.globalCoverage = .5f;
		
		csDef.coverage_broad = Texture(device, "../Textures/worley.png");
		//csDef.coverage_broad.LoadWithMipLevels(device, context, "../Textures/worley.png");
		csDef.coverage_frequent = Texture(device, "../Textures/highDetail2.png");
		csDef.blue_noise = Texture(device, "../Textures/blue_noise_64_tiled.png");
		Create3D(device);

		csDef.scrQuadOffset = 1.f;
		csDef.heightMask = SVec2(400, 600);
		csDef.repeat = SVec4(4096.f, 4069.f, 4069.f, 1.f);

		csDef.baseShape = srv;
	}



	void StrifeLevel::procGen(){}



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

		//csDef.planeMat.m[3][1] = csDef.heightMask.x;

		//SMatrix scrQuadMat = rc.cam->GetCameraMatrix();
		//Math::Translate(scrQuadMat, scrQuadMat.Backward() * csDef.scrQuadOffset);
		//csDef.planeMat = scrQuadMat;
	}



	void StrifeLevel::draw(const RenderContext& rc)
	{
		rc.d3d->ClearColourDepthBuffers();

		//terrain
		shady.light.SetShaderParameters(context, floor.transform, *rc.cam, csDef.celestial, rc.dTime);
		floor.Draw(context, shady.light);
		
		//skybox
		//randy.RenderSkybox(*rc.cam, skybox, skyboxCubeMapper);

		//cloudscape, blend into background which depends on the time of the day... or use anything idk...
		rc.d3d->TurnOnAlphaBlending();

		shady.strife.SetShaderParameters(context, *rc.cam, csDef, rc.elapsed);
		screenQuad.Draw(context, shady.strife);
		shady.strife.ReleaseShaderParameters(context);

		rc.d3d->TurnOffAlphaBlending();

		//GUI
		if(inman.GetMouseMode())
			ToolGUI::Render(csDef);

		rc.d3d->EndScene();
	}

}
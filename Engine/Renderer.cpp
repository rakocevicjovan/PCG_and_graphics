#include "Renderer.h"
#include "SimpleMath.h"
#include "InputManager.h"

Renderer::Renderer() : proceduralTerrain(), perlin(237){
	_D3D = 0;
	drawUI = false;
}


Renderer::~Renderer(){}


bool Renderer::Initialize(int windowWidth, int windowHeight, HWND hwnd, InputManager& inMan)
{
	_inMan = &inMan;
	bool result;

	// Create the Direct3D object.
	_D3D = new D3DClass;
	if(!_D3D)
		return false;

	// Initialize the Direct3D object.
	result = _D3D->Initialize(windowWidth, windowHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result){
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}
	_device = _D3D->GetDevice();
	_deviceContext = _D3D->GetDeviceContext();

	shMan.init(_device, hwnd);

	modBall.LoadModel(_device, "../Models/ball.fbx");
	//Math::Scale(modBall.transform, SVec3(36.f));
	//Math::Translate(modBall.transform, modBallStand.transform.Translation() + SVec3(0.0f, 42.0f, 0.0f));

	///MODEL LOADING
	/*
	modTerrain.LoadModel(_device, "../Models/Terrain/NewTerTex.fbx", 50, 50);
	Math::Scale(modTerrain.transform, SVec3(2.f));
	_terrainModels.push_back(&modTerrain);

	modTreehouse.LoadModel(_device, "../Models/Terrain/Treehouse/thouse(formats).fbx");
	Math::SetTranslation(modTreehouse.transform, SVec3(0.0f, -50.f, -100.f));
	SMatrix treehouseRotation = SMatrix::CreateFromAxisAngle(SVec3::Up, 30.f);
	Math::SetRotation(modTreehouse.transform, treehouseRotation);
	_terrainModels.push_back(&modTreehouse);

	modBallStand.LoadModel(_device, "../Models/ballstand.fbx");
	SMatrix modBallStandRotation = SMatrix::CreateFromAxisAngle(SVec3::Right, PI * 0.5f);
	Math::SetRotation(modBallStand.transform, modBallStandRotation);
	Math::Scale(modBallStand.transform, SVec3(10.f));
	Math::Translate(modBallStand.transform, SVec3(300.0f, -35.0f, -295.0f));

	modDepths.LoadModel(_device, "../Models/WaterQuad.fbx");
	Math::Scale(modDepths.transform, SVec3(120.0f));
	Math::Translate(modDepths.transform, SVec3(0.0f, -50.0f, 0.0f));
	*/

	modStrife.LoadModel(_device, "../Models/WaterQuad.fbx");
	Math::Scale(modStrife.transform, SVec3(15.0f));
	Math::RotateMatByMat(modStrife.transform, SMatrix::CreateFromAxisAngle(SVec3::Right, PI));
	Math::Translate(modStrife.transform, SVec3(-200.f, 200.0f, -200.0f));

	modSkybox.LoadModel(_device, "../Models/Skysphere.fbx");
	Math::Scale(modSkybox.transform, SVec3(10.0f));
	modWaterQuad.LoadModel(_device, "../Models/WaterQuad.fbx");
	
	///Audio
	/*
	musicLSystem.reseed("d");
	musicLSystem.addRule('d', "Fa");
	musicLSystem.addRule('e', "Gb");
	musicLSystem.addRule('f', "Ac");
	musicLSystem.addRule('g', "bD");
	musicLSystem.addRule('a', "cE");
	musicLSystem.addRule('b', "dF");
	musicLSystem.addRule('c', "Eg");

	musicLSystem.addRule('D', "aD");
	musicLSystem.addRule('E', "gB");
	musicLSystem.addRule('F', "aC");
	musicLSystem.addRule('G', "Bd");
	musicLSystem.addRule('A', "Ce");
	musicLSystem.addRule('B', "Df");
	musicLSystem.addRule('C', "eG");

	musicLSystem.rewrite(6);
	
	std::string lSystemNotes = musicLSystem.getString();
	std::vector<std::string> notes;
	for (char lsn : lSystemNotes)
		notes.push_back(std::string(1, lsn));

	audio.init();
	audio.storeSequence(notes);
	*/

	///LIGHT DATA, SHADOW MAP AND UI INITIALISATION
	LightData lightData(SVec3(0.1f, 0.7f, 0.9f), .002f, SVec3(0.8f, 0.8f, 1.0f), .3f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);
	
	pointLight = PointLight(lightData, SVec4(2000.f, 1000.f, 1000.f, 1.0f));	//old moon position SVec4(50.0f, 250.f, 250.0f, 1.0f)

	_rekt = new Rekt(_device, _deviceContext);
	screenRect = _rekt->AddUINODE(_rekt->getRoot(), SVec2(0.75f, 0.75f), SVec2(0.25f, 0.25f));

	SVec3 lookAtPoint = SVec3(0.f, 100.0f, 0.0f);
	SVec3 LVDIR = lookAtPoint - SVec3(pointLight.pos.x, pointLight.pos.y, pointLight.pos.z);
	LVDIR.Normalize();
	SVec3 LVR = LVDIR.Cross(SVec3::Up);
	LVR.Normalize();
	SVec3 LVUP = LVR.Cross(LVDIR);
	LVUP.Normalize();

	dirLight = DirectionalLight(lightData, SVec4(LVDIR.x, LVDIR.y, LVDIR.z, 0.0f));



	offScreenTexture.Init(_device, ostW, ostH);
	offScreenTexture._view = DirectX::XMMatrixLookAtLH(SVec3(pointLight.pos.x, pointLight.pos.y, pointLight.pos.z), lookAtPoint, LVUP);
	offScreenTexture._lens = DirectX::XMMatrixOrthographicLH((float)ostW, (float)ostH, 1.0f, 1000.0f);



	///CAMERA INITIALISATION
	SMatrix projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(_D3D->_fieldOfView, _D3D->_screenAspect, SCREEN_NEAR, SCREEN_DEPTH);

	_cam = Camera(SMatrix::Identity, projectionMatrix);
	_controller = Controller(&inMan);
	_cam._controller = &_controller;
	

	///CUBE MAPS SETUP
	cubeMapper.Init(_device);
	shadowCubeMapper.Init(_device);
	skyboxCubeMapper.LoadFromFiles(_device, "../Textures/night.dds");


	///NOISES
	white.LoadFromFile("../Textures/noiz.png");
	white.Setup(_device);
	perlinTex.LoadFromFile("../Textures/strife.png");
	perlinTex.Setup(_device);
	worley.LoadFromFile("../Textures/worley.png");
	worley.Setup(_device);

	pSys.init(_device, 100, SVec3(0, 0, 100), "../Models/ball.fbx");
	
	lambda = [this](PUD* pud) -> void  
	{
		for (int i = 0; i < pSys._particles.size(); ++i)
		{
			pSys._particles[i]->age += pud->dTime * 0.1f;
			SVec3 translation(pud->windDirection * pud->windVelocity);	// 
			translation.x *= sin(pSys._particles[i]->age * 0.2f * (float)(i + 1));
			translation.y *= cos(pSys._particles[i]->age  * ((float)pSys._particles.size() - (float)i));
			translation.z *= cos(pSys._particles[i]->age * 0.2f * (float)(i + 1));
			Math::SetTranslation(pSys._particles[i]->transform, translation * (float)i * 0.33f);
		}
	};
	
	lambda1 = [this](PUD* pud) -> void
	{
		for (int i = 0; i < pSys._particles.size(); ++i)
		{
			pSys._particles[i]->age += pud->dTime * 0.1f;
			SVec3 translation(pud->windDirection * pud->windVelocity);
			translation.x *= sin(pSys._particles[i]->age * 0.2f * (float)(i + 1));
			translation.y *= cos(pSys._particles[i]->age  * ((float)pSys._particles.size() - (float)i));
			translation.z *= cos(pSys._particles[i]->age * 0.2f * (float)(i + 1));
			Math::SetTranslation(pSys._particles[i]->transform, translation * (float)i * -0.33f);
		}
	};

	pSys.setUpdateFunction(lambda);
	pSys.setShader(&shMan.shaderBase);

	return true;
}


bool Renderer::Frame(float dTime){
	
	_cam.update(dTime);

	ProcessSpecialInput();
	elapsed += dTime;

	//audio.playSequence();
	//audio.update();

	return RenderFrame(dTime);
}



bool Renderer::RenderFrame(float dTime)
{
	_deviceContext->RSSetViewports(1, &_D3D->viewport);
	_D3D->SetBackBufferRenderTarget();
	_D3D->BeginScene(clearColour);


	///RENDERING OLD TERRAIN 
	Math::SetTranslation(modSkybox.transform, _cam.GetCameraMatrix().Translation());

	/*
	for (auto tm : _terrainModels) {
		shaderShadow.SetShaderParameters(_deviceContext, *tm, _cam.GetViewMatrix(), offScreenTexture._view, _cam.GetProjectionMatrix(),
			offScreenTexture._lens, pointLight, _cam.GetCameraMatrix().Translation(), offScreenTexture.srv);
		tm->Draw(_deviceContext, shaderShadow);
		shaderShadow.ReleaseShaderParameters(_deviceContext);
	}
	*/

	SMatrix identityMatrix = SMatrix::Identity;

	if (isTerGenerated) 
	{
		_D3D->TurnOffCulling();
		
		proceduralTerrain.Draw(_deviceContext, shMan.shaderLight,
			identityMatrix, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(),
			pointLight, dTime, _cam.GetCameraMatrix().Translation());
		_D3D->TurnOnCulling();

		linden.draw(_deviceContext, shMan.shaderLight,
			identityMatrix, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(),
			pointLight, dTime, _cam.GetCameraMatrix().Translation());
	}

	PUD pud;
	pud.windDirection = SVec3(-5, 2, 5);
	pud.windVelocity = 1.f;
	pud.dTime = dTime;

	pSys.updateStdFunc(&pud);

	shMan.spl.deltaTime = dTime;
	shMan.spl.deviceContext = _deviceContext;
	shMan.spl.dLight = &pointLight;
	shMan.spl.eyePos = &(_cam.GetCameraMatrix().Translation());
	shMan.spl.model = &pSys._model;
	shMan.spl.proj = &(_cam.GetProjectionMatrix());
	shMan.spl.view = &(_cam.GetViewMatrix());
	
	std::vector<InstanceData> instanceData(100);

	for (int i = 0; i < instanceData.size(); ++i)
		instanceData[i]._m = pSys._particles[i]->transform.Transpose();

	shMan.instancedShader.UpdateInstanceData(instanceData);
	shMan.instancedShader.SetShaderParameters(&shMan.spl);
	modBall.Draw(_deviceContext, shMan.instancedShader);
	shMan.instancedShader.ReleaseShaderParameters(_deviceContext);
	
	/*
	for (int i = 0; i < pSys._particles.size(); ++i) 
	{
		pSys._model.transform = pSys._particles[i]->transform;
		shMan.shaderBase.SetShaderParameters(&shMan.spl);
		pSys.draw(_deviceContext);
		shMan.shaderBase.ReleaseShaderParameters(_deviceContext);
	}
	*/
	

	///rendering water and clouds
	/*
	///RENDERING WATER
	shaderWater.SetShaderParameters(_deviceContext, modDepths, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(),
		dirLight, _cam.GetCameraMatrix().Translation(), dTime, white.srv);
	modDepths.Draw(_deviceContext, shaderWater);
	shaderWater.ReleaseShaderParameters(_deviceContext);

	*/
	///RENDERING CLOUD
	/*
	shaderStrife.SetShaderParameters(_deviceContext, modStrife, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(),
		dirLight, _cam.GetCameraMatrix().Translation(), dTime, white.srv, perlinTex.srv, worley.srv, offScreenTexture._view);
	modStrife.Draw(_deviceContext, shaderStrife);
	shaderStrife.ReleaseShaderParameters(_deviceContext);
	*/

	//_rekt->draw(_deviceContext, shaderHUD, offScreenTexture.srv);

	
	_D3D->TurnOffCulling();
	_D3D->SwitchDepthToLessEquals();
	shMan.shaderSkybox.SetShaderParameters(_deviceContext, modSkybox, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(),
		_cam.GetCameraMatrix().Translation(), dTime, skyboxCubeMapper.cm_srv);
	modSkybox.Draw(_deviceContext, shMan.shaderSkybox);
	shMan.shaderSkybox.ReleaseShaderParameters(_deviceContext);
	_D3D->SwitchDepthToDefault();
	_D3D->TurnOnCulling();
	

	_D3D->EndScene();
	return true;
}



void Renderer::Shutdown() 
{
	if (_D3D) 
	{
		_D3D->Shutdown();
		delete _D3D;
		_D3D = 0;
	}
}



void Renderer::OutputFPS(float dTime) 
{
	std::ostringstream ss;
	ss << "Frame time: " << 1.0f / dTime << "\n";
	std::string s(ss.str());
	OutputDebugStringA(ss.str().c_str());
}



void Renderer::ProcessSpecialInput() 
{
	if (_inMan->IsKeyDown(VK_SPACE)) 
	{

		///TERRAIN GENERATION
		//proceduralTerrain = Procedural::Terrain(256, 256, SVec3(1, 1, 1));

		///Diamond square testing
		//proceduralTerrain.GenWithDS(SVec4(0.f, 10.f, 20.f, 30.f), 7u, 0.6f, 10.f);

		///Cellular automata testing
		//proceduralTerrain.GenWithCA(0.5f, 4);

		///Perlin testing	-SVec3(4, 100, 4) scaling with these fbm settings looks great
		//perlin.generate2DTexturePerlin(512, 512, 64.f, 64.f);
		perlin.generate2DTextureFBM(256, 256, 1, sqrt(3), 4u, 1.f, 1.f, true);
		//perlin.writeToFile("C:\\Users\\metal\\Desktop\\Uni\\test.png");
		proceduralTerrain.setScales(1, 10, 1);
		proceduralTerrain.GenFromTexture(perlin._w, perlin._h, perlin.getFloatVector());


		///Terrain deformation testng
		//proceduralTerrain.fault(SRay(SVec3(25.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), 10.f);
		//proceduralTerrain.TerraSlash(SRay(SVec3(25.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), 6.f, 64, 0.9f);
		//proceduralTerrain.Fault(SRay(SVec3(25.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), 10.f);
		//proceduralTerrain.NoisyFault(SRay(SVec3(25.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), -20.f);
		//proceduralTerrain.NoisyFault(SRay(SVec3(75.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), +15.f);
		//proceduralTerrain.TerraSlash(SRay(SVec3(25.f, 0.f, 0.f), SVec3(1.f, 0.f, 1.f)), 6.f, 64, 0.9f);
		//proceduralTerrain.CircleOfScorn(SVec2(proceduralTerrain.getNumCols() / 2, proceduralTerrain.getNumRows() / 2), 40.f, PI * 0.01337f, 0.5f, 64);
		
		///Voronoi tests
		//Procedural::Voronoi v;
		//v.init(25, proceduralTerrain.getNumCols(), proceduralTerrain.getNumRows());
		//std::vector<SVec2> vertPositions = proceduralTerrain.getHorizontalPositions();
		//v.shatter(vertPositions);

		
		///L-systems testing
		linden.addRule('f', "f[-f]*f[+f][/f]");
		linden.rewrite(1);
		linden.genVerts(0.1f, 0.8f, PI * 0.16666f, PI * 0.16666f);
		linden.setUp(_device);
		
		proceduralTerrain.SetUp(_device);
		isTerGenerated = true;
	}
}








//old scene stuff

/*
///PROJECT TEXTURE
SMatrix texView = DirectX::XMMatrixLookAtLH(SVec3(0.0f, 0.0f, -1.0f), SVec3(0.0f, 0.0f, 0.0f), SVec3::Up);
shaderPT.SetShaderParameters(_deviceContext, modTerrain, cam.GetViewMatrix(), cam.GetViewMatrix(), cam.GetProjectionMatrix(),
							cam.GetProjectionMatrix(), _lights[0], cam.GetCameraMatrix().Translation(), dTime, offScreenTexture.srv);
modTerrain.Draw(_deviceContext, shaderPT);
shaderPT.ReleaseShaderParameters(_deviceContext);
*/

/*

///REFLECTION CUBE MAP START
_deviceContext->RSSetViewports(1, &(cubeMapper.cm_viewport));
cubeMapper.UpdateCams(modBall.transform.Translation());

SMatrix wot = DirectX::XMMatrixInverse(nullptr, cubeMapper.cameras[0]);
Math::SetTranslation(modSkybox.transform, wot.Translation());
for (int i = 0; i < 6; i++) {

	_deviceContext->ClearRenderTargetView(cubeMapper.cm_rtv[i], cubeMapper.clearCol);
	_deviceContext->ClearDepthStencilView(cubeMapper.cm_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	_deviceContext->OMSetRenderTargets(1, &cubeMapper.cm_rtv[i], cubeMapper.cm_depthStencilView);

	shaderLight.SetShaderParameters(_deviceContext, modTerrain, cubeMapper.cameras[i], cubeMapper.lens, pointLight, _cam.GetCameraMatrix().Translation(), dTime);
	modTerrain.Draw(_deviceContext, shaderLight);
	shaderLight.ReleaseShaderParameters(_deviceContext);

	shaderLight.SetShaderParameters(_deviceContext, modTreehouse, cubeMapper.cameras[i], cubeMapper.lens, pointLight, _cam.GetCameraMatrix().Translation(), dTime);
	modTreehouse.Draw(_deviceContext, shaderLight);
	shaderLight.ReleaseShaderParameters(_deviceContext);

	_D3D->TurnOffCulling();
	_D3D->SwitchDepthToLessEquals();


	shaderSkybox.SetShaderParameters(_deviceContext, modSkybox, cubeMapper.cameras[i], cubeMapper.lens,
		_cam.GetCameraMatrix().Translation(), dTime, skyboxCubeMapper.cm_srv);
	modSkybox.Draw(_deviceContext, shaderSkybox);
	shaderSkybox.ReleaseShaderParameters(_deviceContext);

	_D3D->SwitchDepthToDefault();
	_D3D->TurnOnCulling();
}
Math::SetTranslation(modSkybox.transform, _cam.GetCameraMatrix().Translation());
///REFLECTION CUBE MAP DONE


///RENDERING UI
_deviceContext->RSSetViewports(1, &_D3D->viewport);
_D3D->SetBackBufferRenderTarget();
_D3D->BeginScene(clearColour);

if (drawUI)
_rekt->draw(_deviceContext, shaderHUD, offScreenTexture.srv);
///RENDERING UI DONE


///RENDERING DEPTH TEXTURE
_deviceContext->RSSetViewports(1, &altViewport);	//to the shadow texture viewport
_deviceContext->OMSetRenderTargets(1, &(offScreenTexture.rtv), _D3D->GetDepthStencilView());	//switch to drawing on ost for the prepass	
_deviceContext->ClearRenderTargetView(offScreenTexture.rtv, ccb);	//then clear it, both the colours and the depth-stencil buffer
_deviceContext->ClearDepthStencilView(_D3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

for (auto tm : _terrainModels) {
	shaderDepth.SetShaderParameters(_deviceContext, *tm, offScreenTexture._view, offScreenTexture._lens);
	tm->Draw(_deviceContext, shaderDepth);
}

shaderLight.SetShaderParameters(_deviceContext, modBall, offScreenTexture._view, offScreenTexture._lens, pointLight,
	_cam.GetCameraMatrix().Translation(), dTime);
modBall.Draw(_deviceContext, shaderLight);
shaderLight.ReleaseShaderParameters(_deviceContext);

///RENDERING TERRAIN
_deviceContext->ClearDepthStencilView(_D3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
_deviceContext->RSSetViewports(1, &_D3D->viewport);
_D3D->SetBackBufferRenderTarget();

for (auto tm : _terrainModels) {
	shaderShadow.SetShaderParameters(_deviceContext, *tm, _cam.GetViewMatrix(), offScreenTexture._view, _cam.GetProjectionMatrix(),
		offScreenTexture._lens, pointLight, _cam.GetCameraMatrix().Translation(), offScreenTexture.srv);
	tm->Draw(_deviceContext, shaderShadow);
	shaderShadow.ReleaseShaderParameters(_deviceContext);
}
///RENDERING TERRAIN DONE

///RENDERING WIREFRAME
_D3D->TurnOnAlphaBlending();
shaderWireframe.SetShaderParameters(_deviceContext, modBallStand, _cam.GetViewMatrix(), _cam.GetProjectionMatrix());
modBallStand.Draw(_deviceContext, shaderWireframe);
shaderWireframe.ReleaseShaderParameters(_deviceContext);
_D3D->TurnOffAlphaBlending();
///RENDERING WIREFRAME DONE




///RENDERING REFLECTION SPHERE/*
shaderCM.SetShaderParameters(_deviceContext, modBall, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(), dirLight,
	_cam.GetCameraMatrix().Translation(), dTime, cubeMapper.cm_srv);
modBall.Draw(_deviceContext, shaderCM);
shaderCM.ReleaseShaderParameters(_deviceContext);
///RENDERING REFLECTION SPHERE DONE



///RENDERING SKYBOX
_D3D->TurnOffCulling();
_D3D->SwitchDepthToLessEquals();
shaderSkybox.SetShaderParameters(_deviceContext, modSkybox, _cam.GetViewMatrix(), _cam.GetProjectionMatrix(),
	_cam.GetCameraMatrix().Translation(), dTime, skyboxCubeMapper.cm_srv);
modSkybox.Draw(_deviceContext, shaderSkybox);
shaderSkybox.ReleaseShaderParameters(_deviceContext);
_D3D->SwitchDepthToDefault();
_D3D->TurnOnCulling();
///RENDERING SKYBOX DONE


_D3D->TurnOnAlphaBlending();





_D3D->TurnOffAlphaBlending();

*/
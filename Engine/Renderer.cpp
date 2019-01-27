#include "Renderer.h"
#include "SimpleMath.h"
#include "InputManager.h"

Renderer::Renderer() : proceduralTerrain(){
	_D3D = 0;
	drawUI = false;
}


Renderer::~Renderer(){}


bool Renderer::Initialize(int windowWidth, int windowHeight, HWND hwnd, InputManager& inMan){
	
	bool result;

	// Create the Direct3D object.
	_D3D = new D3DClass;
	if(!_D3D){
		return false;
	}

	// Initialize the Direct3D object.
	result = _D3D->Initialize(windowWidth, windowHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result){
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}
	_device = _D3D->GetDevice();
	_deviceContext = _D3D->GetDeviceContext();


	//model and shader loading should not be here... but time waits for no man...
	///SHADER LOADING 
	std::vector<std::wstring> names;
	names.push_back(L"lightvs.hlsl");
	names.push_back(L"lightps.hlsl");
	shaderLight.Initialize(_device, hwnd, names);
	_shaders.push_back(shaderLight);

	std::vector<std::wstring> wfsNames;
	wfsNames.push_back(L"wireframe.vs");
	wfsNames.push_back(L"wireframe.gs");
	wfsNames.push_back(L"wireframe.ps");
	shaderWireframe.Initialize(_device, hwnd, wfsNames);

	std::vector<std::wstring> hudNames;
	hudNames.push_back(L"rekt.vs");
	hudNames.push_back(L"rekt.ps");
	shaderHUD.Initialize(_device, hwnd, hudNames);

	std::vector<std::wstring> depthNames;
	depthNames.push_back(L"depth.vs");
	depthNames.push_back(L"depth.ps");
	shaderDepth.Initialize(_device, hwnd, depthNames);

	std::vector<std::wstring> projTexNames;
	projTexNames.push_back(L"projectTex.vs");
	projTexNames.push_back(L"projectTex.ps");
	shaderPT.Initialize(_device, hwnd, projTexNames);

	std::vector<std::wstring> shadowNames;
	shadowNames.push_back(L"shadowvs.hlsl");
	shadowNames.push_back(L"shadowps.hlsl");
	shaderShadow.Initialize(_device, hwnd, shadowNames);

	std::vector<std::wstring> cubeMapNames;
	cubeMapNames.push_back(L"cubemap.vs");
	cubeMapNames.push_back(L"cubemap.ps");
	shaderCM.Initialize(_device, hwnd, cubeMapNames);

	std::vector<std::wstring> skyboxNames;
	skyboxNames.push_back(L"skyboxvs.hlsl");
	skyboxNames.push_back(L"skyboxps.hlsl");
	shaderSkybox.Initialize(_device, hwnd, skyboxNames);

	std::vector<std::wstring> strifeNames;
	strifeNames.push_back(L"strifevs.hlsl");
	strifeNames.push_back(L"strifeps.hlsl");
	shaderStrife.Initialize(_device, hwnd, strifeNames);

	std::vector<std::wstring> waterNames;
	waterNames.push_back(L"Watervs.hlsl");
	waterNames.push_back(L"Waterps.hlsl");
	shaderWater.Initialize(_device, hwnd, waterNames);
	///SHADER LOADING DONE



	///MODEL LOADING
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

	modBall.LoadModel(_device, "../Models/ball.fbx");
	Math::Scale(modBall.transform, SVec3(36.f));
	Math::Translate(modBall.transform, modBallStand.transform.Translation() + SVec3(0.0f, 42.0f, 0.0f));

	modStrife.LoadModel(_device, "../Models/oohlala.fbx");
	Math::Scale(modStrife.transform, SVec3(150.0f));
	Math::Translate(modStrife.transform, SVec3(-700.f, 200.0f, -700.0f));

	modDepths.LoadModel(_device, "../Models/WaterQuad.fbx");
	Math::Scale(modDepths.transform, SVec3(120.0f));
	Math::Translate(modDepths.transform, SVec3(0.0f, -50.0f, 0.0f));

	modSkybox.LoadModel(_device, "../Models/Skysphere.fbx");
	Math::Scale(modSkybox.transform, SVec3(10.0f));
	modWaterQuad.LoadModel(_device, "../Models/WaterQuad.fbx");
	///MODEL LOADING DONE


	///LIGHT DATA, SHADOW MAP AND UI INITIALISATION
	LightData lightData(SVec3(0.6f, 0.7f, 0.9f), .002f, SVec3(0.8f, 0.8f, 1.0f), .3f, SVec3(0.3f, 0.5f, 1.0f), 0.7f);
	
	pointLight = PointLight(lightData, SVec4(10000.f, 0000.f, 0000.f, 1.0f));	//old moon position SVec4(50.0f, 250.f, 250.0f, 1.0f)

	_rekt = new Rekt(_device, _deviceContext);
	screenRect = _rekt->AddUINODE(_rekt->getRoot(), SVec2(0.75f, 0.75f), SVec2(0.25f, 0.25f));

	SVec3 lookAtPoint = SVec3(0.f, 100.0f, 0.0f);
	SVec3 LVDIR = lookAtPoint - SVec3(pointLight.pos.x, pointLight.pos.y, pointLight.pos.z);
	LVDIR.Normalize();
	SVec3 LVR = LVDIR.Cross(SVec3::Up);
	LVR.Normalize();
	SVec3 LVUP = LVR.Cross(LVDIR);
	LVUP.Normalize();

	offScreenTexture.Init(_device, ostW, ostH);
	offScreenTexture._view = DirectX::XMMatrixLookAtLH(SVec3(pointLight.pos.x, pointLight.pos.y, pointLight.pos.z), lookAtPoint, LVUP);
	offScreenTexture._lens = DirectX::XMMatrixOrthographicLH((float)ostW, (float)ostH, 1.0f, 1000.0f);

	dirLight = DirectionalLight(lightData, SVec4(LVDIR.x, LVDIR.y, LVDIR.z, 0.0f));
	///LIGHT DATA, SHADOW MAP AND UI INITIALISATION DONE



	///CAMERA INITIALISATION
	SMatrix projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(_D3D->_fieldOfView, _D3D->_screenAspect, SCREEN_NEAR, SCREEN_DEPTH);

	Camera cam(SMatrix::Identity, projectionMatrix);
	_controllers.push_back(Controller(&inMan));
	cam._controller = &_controllers[0];
	_cameras.push_back(cam);
	///CAMERA INITIALISATION DONE



	///OFF SCREEN TEXTURE VIEWPORT SETUP
	altViewport.Width = (float)ostW;
	altViewport.Height = (float)ostH;
	altViewport.MinDepth = 0.0f;
	altViewport.MaxDepth = 1.0f;
	altViewport.TopLeftX = 0.0f;
	altViewport.TopLeftY = 0.0f;
	///OFF SCREEN TEXTURE VIEWPORT SETUP DONE
	


	///CUBE MAPS SETUP
	cubeMapper.Init(_device);
	shadowCubeMapper.Init(_device);
	skyboxCubeMapper.LoadFromFiles(_device, "../Textures/night.dds");
	///CUBE MAPS SETUP DONE



	///NOISES
	white.LoadFromFile("../Textures/noiz.png");
	white.Setup(_device);
	perlin.LoadFromFile("../Textures/strife.png");
	perlin.Setup(_device);
	worley.LoadFromFile("../Textures/worley.png");
	worley.Setup(_device);
	///NOISES DONE



	///TERRAIN GENERATION
	proceduralTerrain.setScales(30, 10, 30);
	proceduralTerrain = Procedural::Terrain(30, 30);
	proceduralTerrain.GenWithCA(40);
	proceduralTerrain.SetUp(_device);

	/* //heightmap example 
	Texture t;
	t.fileName = "../Textures/volcano.png";
	BitMapper bitMapper(t);
	bitMapper.init(1, 2, 2, 0.3);	//g, w, l, h
	if (bitMapper.createTerrain())
		bitMapper.terrainToFile("../Models/Terrain/newTerrain.obj");
	else
		std::cout << " Failed to create terrain." << std::endl;
	*/
	///TERRAIN GENERATION DONE

	return true;
}



bool Renderer::Frame(float dTime){
	
	for (Camera& c : _cameras)
		c.update(dTime);

	//OutputFPS(dTime);

	return RenderFrame(dTime);
}



bool Renderer::RenderFrame(float dTime){

	_D3D->BeginScene(clearColour);


	///RENDERING OLD TERRAIN 
	Math::SetTranslation(modSkybox.transform, _cameras[0].GetCameraMatrix().Translation());

	_deviceContext->ClearDepthStencilView(_D3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	_deviceContext->RSSetViewports(1, &_D3D->viewport);
	_D3D->SetBackBufferRenderTarget();

	/*
	for (auto tm : _terrainModels) {
		shaderShadow.SetShaderParameters(_deviceContext, *tm, _cameras[0].GetViewMatrix(), offScreenTexture._view, _cameras[0].GetProjectionMatrix(),
			offScreenTexture._lens, pointLight, _cameras[0].GetCameraMatrix().Translation(), offScreenTexture.srv);
		tm->Draw(_deviceContext, shaderShadow);
		shaderShadow.ReleaseShaderParameters(_deviceContext);
	}
	*/

	//_D3D->TurnOffCulling();
	SMatrix identityMatrix = SMatrix::Identity;
	proceduralTerrain.Draw(_deviceContext, shaderLight, 
		identityMatrix, _cameras[0].GetViewMatrix(), _cameras[0].GetProjectionMatrix(),
		pointLight, dTime, _cameras[0].GetCameraMatrix().Translation());
	//_D3D->TurnOnCulling();

	/*
	_D3D->TurnOffCulling();
	_D3D->SwitchDepthToLessEquals();
	shaderSkybox.SetShaderParameters(_deviceContext, modSkybox, _cameras[0].GetViewMatrix(), _cameras[0].GetProjectionMatrix(),
		_cameras[0].GetCameraMatrix().Translation(), dTime, skyboxCubeMapper.cm_srv);
	modSkybox.Draw(_deviceContext, shaderSkybox);
	shaderSkybox.ReleaseShaderParameters(_deviceContext);
	_D3D->SwitchDepthToDefault();
	_D3D->TurnOnCulling();
	*/
	_D3D->EndScene();
	return true;
}



Camera& Renderer::addCamera(SMatrix& camTransform, SMatrix& lens) {
	_cameras.push_back(Camera(camTransform, lens));
	return _cameras.back();
}



Shader& Renderer::addShader() {
	_shaders.push_back(Shader());
	return _shaders.back();
}



void Renderer::Shutdown() {

	if (_D3D) {
		_D3D->Shutdown();
		delete _D3D;
		_D3D = 0;
	}

	return;
}



void Renderer::OutputFPS(float dTime) {
	std::ostringstream ss;
	ss << "Frame time: " << 1.0f / dTime << "\n";
	std::string s(ss.str());
	OutputDebugStringA(ss.str().c_str());
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

	shaderLight.SetShaderParameters(_deviceContext, modTerrain, cubeMapper.cameras[i], cubeMapper.lens, pointLight, _cameras[0].GetCameraMatrix().Translation(), dTime);
	modTerrain.Draw(_deviceContext, shaderLight);
	shaderLight.ReleaseShaderParameters(_deviceContext);

	shaderLight.SetShaderParameters(_deviceContext, modTreehouse, cubeMapper.cameras[i], cubeMapper.lens, pointLight, _cameras[0].GetCameraMatrix().Translation(), dTime);
	modTreehouse.Draw(_deviceContext, shaderLight);
	shaderLight.ReleaseShaderParameters(_deviceContext);

	_D3D->TurnOffCulling();
	_D3D->SwitchDepthToLessEquals();


	shaderSkybox.SetShaderParameters(_deviceContext, modSkybox, cubeMapper.cameras[i], cubeMapper.lens,
		_cameras[0].GetCameraMatrix().Translation(), dTime, skyboxCubeMapper.cm_srv);
	modSkybox.Draw(_deviceContext, shaderSkybox);
	shaderSkybox.ReleaseShaderParameters(_deviceContext);

	_D3D->SwitchDepthToDefault();
	_D3D->TurnOnCulling();
}
Math::SetTranslation(modSkybox.transform, _cameras[0].GetCameraMatrix().Translation());
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
	_cameras[0].GetCameraMatrix().Translation(), dTime);
modBall.Draw(_deviceContext, shaderLight);
shaderLight.ReleaseShaderParameters(_deviceContext);
///RENDERING DEPTH TEXTURE DONE

///RENDERING TERRAIN
_deviceContext->ClearDepthStencilView(_D3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
_deviceContext->RSSetViewports(1, &_D3D->viewport);
_D3D->SetBackBufferRenderTarget();

for (auto tm : _terrainModels) {
	shaderShadow.SetShaderParameters(_deviceContext, *tm, _cameras[0].GetViewMatrix(), offScreenTexture._view, _cameras[0].GetProjectionMatrix(),
		offScreenTexture._lens, pointLight, _cameras[0].GetCameraMatrix().Translation(), offScreenTexture.srv);
	tm->Draw(_deviceContext, shaderShadow);
	shaderShadow.ReleaseShaderParameters(_deviceContext);
}
///RENDERING TERRAIN DONE

///RENDERING WIREFRAME
_D3D->TurnOnAlphaBlending();
shaderWireframe.SetShaderParameters(_deviceContext, modBallStand, _cameras[0].GetViewMatrix(), _cameras[0].GetProjectionMatrix());
modBallStand.Draw(_deviceContext, shaderWireframe);
shaderWireframe.ReleaseShaderParameters(_deviceContext);
_D3D->TurnOffAlphaBlending();
///RENDERING WIREFRAME DONE




///RENDERING REFLECTION SPHERE/*
shaderCM.SetShaderParameters(_deviceContext, modBall, _cameras[0].GetViewMatrix(), _cameras[0].GetProjectionMatrix(), dirLight,
	_cameras[0].GetCameraMatrix().Translation(), dTime, cubeMapper.cm_srv);
modBall.Draw(_deviceContext, shaderCM);
shaderCM.ReleaseShaderParameters(_deviceContext);
///RENDERING REFLECTION SPHERE DONE



///RENDERING SKYBOX
_D3D->TurnOffCulling();
_D3D->SwitchDepthToLessEquals();
shaderSkybox.SetShaderParameters(_deviceContext, modSkybox, _cameras[0].GetViewMatrix(), _cameras[0].GetProjectionMatrix(),
	_cameras[0].GetCameraMatrix().Translation(), dTime, skyboxCubeMapper.cm_srv);
modSkybox.Draw(_deviceContext, shaderSkybox);
shaderSkybox.ReleaseShaderParameters(_deviceContext);
_D3D->SwitchDepthToDefault();
_D3D->TurnOnCulling();
///RENDERING SKYBOX DONE


_D3D->TurnOnAlphaBlending();

///RENDERING CLOUD
shaderStrife.SetShaderParameters(_deviceContext, modStrife, _cameras[0].GetViewMatrix(), _cameras[0].GetProjectionMatrix(),
	dirLight, _cameras[0].GetCameraMatrix().Translation(), dTime, white.srv, perlin.srv, worley.srv, offScreenTexture._view);
modStrife.Draw(_deviceContext, shaderStrife);
shaderStrife.ReleaseShaderParameters(_deviceContext);
///RENDERING CLOUD DONE

///RENDERING WATER
shaderWater.SetShaderParameters(_deviceContext, modDepths, _cameras[0].GetViewMatrix(), _cameras[0].GetProjectionMatrix(),
	dirLight, _cameras[0].GetCameraMatrix().Translation(), dTime, white.srv);
modDepths.Draw(_deviceContext, shaderWater);
shaderWater.ReleaseShaderParameters(_deviceContext);
///RENDERING WATER DONE

_D3D->TurnOffAlphaBlending();

*/
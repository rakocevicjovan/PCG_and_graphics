#include "Renderer.h"
#include "SimpleMath.h"
#include "InputManager.h"

Renderer::Renderer(){
	_D3D = 0;
	drawUI = true;
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


	//this should not be here... but whatever...
	std::vector<std::wstring> names;
	names.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/lightvs.hlsl");
	names.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/lightps.hlsl");
	shaderLight.Initialize(_device, hwnd, names);
	_shaders.push_back(shaderLight);

	std::vector<std::wstring> wfsNames;
	wfsNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/wireframe.vs");
	wfsNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/wireframe.gs");
	wfsNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/wireframe.ps");
	wfs.Initialize(_device, hwnd, wfsNames);

	std::vector<std::wstring> hudNames;
	hudNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/rekt.vs");
	hudNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/rekt.ps");
	shaderHUD.Initialize(_device, hwnd, hudNames);

	std::vector<std::wstring> depthNames;
	depthNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/depth.vs");
	depthNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/depth.ps");
	shaderDepth.Initialize(_device, hwnd, depthNames);

	std::vector<std::wstring> projTexNames;
	projTexNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/projectTex.vs");
	projTexNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/projectTex.ps");
	shaderPT.Initialize(_device, hwnd, projTexNames);

	std::vector<std::wstring> shadowNames;
	shadowNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/shadowvs.hlsl");
	shadowNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/shadowps.hlsl");
	shaderShadow.Initialize(_device, hwnd, shadowNames);

	std::vector<std::wstring> cubeMapNames;
	cubeMapNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/cubemap.vs");
	cubeMapNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/cubemap.ps");
	shaderCM.Initialize(_device, hwnd, cubeMapNames);

	std::vector<std::wstring> skyboxNames;
	skyboxNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/skyboxvs.hlsl");
	skyboxNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/skyboxps.hlsl");
	shaderSkybox.Initialize(_device, hwnd, skyboxNames);

	std::vector<std::wstring> strifeNames;
	strifeNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/strifevs.hlsl");
	strifeNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/strifeps.hlsl");
	shaderStrife.Initialize(_device, hwnd, strifeNames);




	//NewTerTex.fbx	, 50, 50
	modTerrain.LoadModel(_device, "C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Models/Terrain/NewTerTex.fbx", 50, 50);
	Math::Scale(modTerrain.transform, SVec3(2.f));
	_terrainModels.push_back(&modTerrain);


	modTreehouse.LoadModel(_device, "C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Models/Terrain/Treehouse/thouse(formats).fbx");
	Math::SetTranslation(modTreehouse.transform, SVec3(0.0f, -50.f, -100.f));
	SMatrix treehouseRotation = SMatrix::CreateFromAxisAngle(SVec3::Up, 30.f);
	Math::SetRotation(modTreehouse.transform, treehouseRotation);
	_terrainModels.push_back(&modTreehouse);


	modBall.LoadModel(_device, "C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Models/ball.fbx");	
	Math::Scale(modBall.transform, SVec3(10.f));
	Math::Translate(modBall.transform, SVec3(0.0f, 300.0f, -300.0f));
	


	modSkybox.LoadModel(_device, "C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Models/Skysphere.fbx");
	modWaterQuad.LoadModel(_device, "C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Models/WaterQuad.fbx");

	LightData lightData(SVec3(0.6f, 0.7f, 0.9f), .01f, SVec3(0.8f, 0.8f, 1.0f), .4f, SVec3(0.3f, 0.5f, 1.0f), 1.f);
	
	pointLight = PointLight(lightData, SVec4(50.0f, 250.f, 250.0f, 1.0f));
	SVec4 tmpDir = SVec4(-pointLight.pos.x, -pointLight.pos.y, -pointLight.pos.z, 0.0f);
	tmpDir.Normalize();
	dirLight = DirectionalLight(lightData, tmpDir);

	SMatrix projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(_D3D->_fieldOfView, _D3D->_screenAspect, SCREEN_NEAR, SCREEN_DEPTH);

	Camera cam(SMatrix::Identity, projectionMatrix);
	_controllers.push_back(Controller(&inMan));
	cam._controller = &_controllers[0];
	_cameras.push_back(cam);

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
	//offScreenTexture._lens = DirectX::XMMatrixPerspectiveFovLH(offScreenTexture._fov, offScreenTexture._ar, 0.1f, 700.0f);
	offScreenTexture._lens = DirectX::XMMatrixOrthographicLH((float)ostW, (float)ostH, 1.0f, 1000.0f);

	altViewport.Width = (float)ostW;
	altViewport.Height = (float)ostH;
	altViewport.MinDepth = 0.0f;
	altViewport.MaxDepth = 1.0f;
	altViewport.TopLeftX = 0.0f;
	altViewport.TopLeftY = 0.0f;

	//cubeMapper.edgeLength = 512;
	cubeMapper.Init(_device);
	shadowCubeMapper.Init(_device);
	skyboxCubeMapper.LoadFromFiles(_device, "C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Textures/night.dds");

	/*//the bitmapper
	Texture t;
	t.fileName = "C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Textures/volcano.png";
	BitMapper bitMapper(t);
	bitMapper.init(1, 2, 2, 0.3);	//g, w, l, h
	if (bitMapper.createTerrain())
		bitMapper.terrainToFile("../Models/Terrain/newTerrain.obj");
	else
		std::cout << " Failed to create terrain." << std::endl;
	*/

	return true;
}



bool Renderer::Frame(float dTime){
	
	for (Camera& c : _cameras)
		c.update(dTime);

	Math::SetTranslation(modSkybox.transform, _cameras[0].GetCameraMatrix().Translation());

	OutputFPS(dTime);

	return RenderFrame(dTime);
}



bool Renderer::RenderFrame(float dTime){

	///CUBE SHADOWS
/*
	_deviceContext->RSSetViewports(1, &(shadowCubeMapper.cm_viewport));
	shadowCubeMapper.UpdateCams(SVec3(pLight.pos));

	for (int i = 0; i < 6; i++) {
		//use depth shader here...
		_deviceContext->ClearRenderTargetView(shadowCubeMapper.cm_rtv[i], shadowCubeMapper.clearCol);
		_deviceContext->ClearDepthStencilView(shadowCubeMapper.cm_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		_deviceContext->OMSetRenderTargets(1, &shadowCubeMapper.cm_rtv[i], shadowCubeMapper.cm_depthStencilView);

		for (auto model : models) {
			shaderDepth.SetShaderParameters(_deviceContext, *model, cubeMapper.cameras[i], cubeMapper.lens);
			model->Draw(_deviceContext, shaderDepth);
		}
	}
*/
	///CUBE SHADOWS DONE

	///REFLECTION CUBE MAP START
	_deviceContext->RSSetViewports(1, &(cubeMapper.cm_viewport));
	cubeMapper.UpdateCams(modBall.transform.Translation());

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

	}
	///REFLECTION CUBE MAP DONE

	_deviceContext->RSSetViewports(1, &_D3D->viewport);
	_D3D->SetBackBufferRenderTarget();
	_D3D->BeginScene(clearColour);

	if (drawUI)
		_rekt->draw(_deviceContext, shaderHUD, offScreenTexture.srv);


	/*depth texture*/
	_deviceContext->RSSetViewports(1, &altViewport);	//to the shadow texture viewport
	_deviceContext->OMSetRenderTargets(1, &(offScreenTexture.rtv), _D3D->GetDepthStencilView());	//switch to drawing on ost for the prepass	
	_deviceContext->ClearRenderTargetView(offScreenTexture.rtv, ccb);	//then clear it, both the colours and the depth-stencil buffer
	_deviceContext->ClearDepthStencilView(_D3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	for (auto tm : _terrainModels) {
		shaderDepth.SetShaderParameters(_deviceContext, *tm, offScreenTexture._view, offScreenTexture._lens);
		tm->Draw(_deviceContext, shaderDepth);
	}

	_deviceContext->ClearDepthStencilView(_D3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	_deviceContext->RSSetViewports(1, &_D3D->viewport);
	_D3D->SetBackBufferRenderTarget();

	for (auto tm : _terrainModels) {
		/**/
		shaderShadow.SetShaderParameters(_deviceContext, *tm, _cameras[0].GetViewMatrix(), offScreenTexture._view, _cameras[0].GetProjectionMatrix(),
			offScreenTexture._lens, pointLight, _cameras[0].GetCameraMatrix().Translation(), offScreenTexture.srv);
		tm->Draw(_deviceContext, shaderShadow);
		shaderShadow.ReleaseShaderParameters(_deviceContext);
		/**/
		/*
		shaderLight.SetShaderParameters(_deviceContext, *tm, _cameras[0].GetViewMatrix(), _cameras[0].GetProjectionMatrix(), pointLight,
			_cameras[0].GetCameraMatrix().Translation(), dTime);
		tm->Draw(_deviceContext, shaderLight);
		shaderLight.ReleaseShaderParameters(_deviceContext);
		/**/
	}

	
	shaderCM.SetShaderParameters(_deviceContext, modBall, _cameras[0].GetViewMatrix(), _cameras[0].GetProjectionMatrix(), dirLight,
		_cameras[0].GetCameraMatrix().Translation(), dTime, cubeMapper.cm_srv);
	modBall.Draw(_deviceContext, shaderCM);
	shaderCM.ReleaseShaderParameters(_deviceContext);
	

	//skybox
	_D3D->TurnOffCulling();
	_D3D->SwitchDepthToLessEquals();
	shaderSkybox.SetShaderParameters(_deviceContext, modSkybox, _cameras[0].GetViewMatrix(), _cameras[0].GetProjectionMatrix(),
		_cameras[0].GetCameraMatrix().Translation(), dTime, skyboxCubeMapper.cm_srv);
	modSkybox.Draw(_deviceContext, shaderSkybox);
	shaderSkybox.ReleaseShaderParameters(_deviceContext);
	_D3D->TurnOnCulling();
	_D3D->SwitchDepthToDefault();
	//skybox end

	///PROJECT TEXTURE
	/*
	SMatrix texView = DirectX::XMMatrixLookAtLH(SVec3(0.0f, 0.0f, -1.0f), SVec3(0.0f, 0.0f, 0.0f), SVec3::Up);
	shaderPT.SetShaderParameters(_deviceContext, modTerrain, cam.GetViewMatrix(), cam.GetViewMatrix(), cam.GetProjectionMatrix(),
								cam.GetProjectionMatrix(), _lights[0], cam.GetCameraMatrix().Translation(), dTime, offScreenTexture.srv);
	modTerrain.Draw(_deviceContext, shaderPT);
	shaderPT.ReleaseShaderParameters(_deviceContext);
	*/
	///PROJECT TEXTURE END

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
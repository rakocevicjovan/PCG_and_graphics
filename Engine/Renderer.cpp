#include "Renderer.h"
#include "SimpleMath.h"
#include "InputManager.h"

Renderer::Renderer(){
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


	//this should not be here... but whatever...
	std::vector<std::wstring> names;
	names.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/light.vs");
	names.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/light.ps");
	
	Shader joeSchmoe;
	joeSchmoe.Initialize(_device, hwnd, names);
	_shaders.push_back(joeSchmoe);

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
	shadowNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/shadow.vs");
	shadowNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/shadow.ps");
	shaderShadow.Initialize(_device, hwnd, shadowNames);

	std::vector<std::wstring> cubeMapNames;
	cubeMapNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/cubemap.vs");
	cubeMapNames.push_back(L"C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Engine/cubemap.ps");
	shaderCM.Initialize(_device, hwnd, cubeMapNames);

	//mountain.obj
	mod.LoadModel(_device, "C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Models/Terrain/Treehouse/thouse(formats).fbx");
	//mod.LoadModel(_device, "C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Models/Terrain/mountain.obj");
	_models.push_back(&mod);

	///Dragon/Dragon 2o5_fbx.fbx
	mod2.LoadModel(_device, "C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Models/ball.fbx");	
	Math::Translate(mod2.transform, SVec3(0.0f, 20.0f, 15.0f));
	_models.push_back(&mod2);


	LightData ld(SVec3(1.0f), .1f, SVec3(1.0f), .3f, SVec3(1.0f), 0.5f);
	
	DirectionalLight light(ld, SVec4(0.0f, 0.0f, 1.0f, 1.0f));	//SVec4(0.0f, .707f, .707f, 1.f)
	_lights.push_back(light);

	pLight = PointLight(ld, SVec4(0.0f, 20.f, -20.0f, 1.0f));

	SMatrix cMat;
	//cMat = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&eyePos), SVec3(0.0f, 0.0f, 0.0f), SVec3::Up);

	SMatrix projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(_D3D->_fieldOfView, _D3D->_screenAspect, SCREEN_NEAR, SCREEN_DEPTH);

	Camera cam(cMat, projectionMatrix);

	_controllers.push_back(Controller(&inMan));

	cam._controller = &_controllers[0];

	_cameras.push_back(cam);

	_rekt = new Rekt(_device, _deviceContext);
	screenRect = _rekt->AddUINODE(_rekt->getRoot(), SVec2(0.75f, 0.75f), SVec2(0.25f, 0.25f));
	offScreenTexture.Init(_device, ostW, ostH);

	_lightvm = DirectX::XMMatrixLookAtLH(SVec3(pLight.pos.x, pLight.pos.y, pLight.pos.z), SVec3(0.0f, 0.0f, 0.0f), SVec3::Forward);
	_lightpm = DirectX::XMMatrixPerspectiveFovLH(_D3D->_fieldOfView, _D3D->_screenAspect, 0.1f, 200.0f);
	//_lightpm = DirectX::XMMatrixOrthographicLH(ostW, ostH, 0.1f, 200.0f);

	altViewport.Width = (float)800;
	altViewport.Height = (float)600;
	altViewport.MinDepth = 0.0f;
	altViewport.MaxDepth = 1.0f;
	altViewport.TopLeftX = 0.0f;
	altViewport.TopLeftY = 0.0f;

	//cubeMapper.edgeLength = 512;
	cubeMapper.Init(_device);
	shadowCubeMapper.Init(_device);

	/*
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

	for (Camera& c : _cameras) {
		c.update(dTime);
	}

	/*std::ostringstream ss;
	ss << "Frame time: " << dTime << "\n";
	std::string s(ss.str());
	OutputDebugStringA(ss.str().c_str());*/

	return RenderFrame(_models, _cameras[0], dTime);
}



bool Renderer::RenderFrame(const std::vector<Model*>& models, const Camera& cam, float dTime){

	/*	//I am now using depth cube map instead of a single depth texture for shadows
	
	_deviceContext->RSSetViewports(1, &altViewport);	//to the small viewport
	_deviceContext->OMSetRenderTargets(1, &(offScreenTexture.rtv), _D3D->GetDepthStencilView());	//switch to drawing on ost for the prepass	
	_deviceContext->ClearRenderTargetView(offScreenTexture.rtv, ccb);	//then clear it, both the colours and the depth-stencil buffer
	_deviceContext->ClearDepthStencilView(_D3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	for (auto model : models) {
		shaderDepth.SetShaderParameters(_deviceContext, *model, _lightvm, _lightpm);
		model->Draw(_deviceContext, shaderDepth);
	}

	_D3D->SetBackBufferRenderTarget();
	*/


	/*
//reflection cube map begin
	_deviceContext->RSSetViewports(1, &(cubeMapper.cm_viewport));
	cubeMapper.UpdateCams(_models[1]->transform.Translation());

	for (int i = 0; i < 6; i++) {

		_deviceContext->ClearRenderTargetView(cubeMapper.cm_rtv[i], cubeMapper.clearCol);
		_deviceContext->ClearDepthStencilView(cubeMapper.cm_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		_deviceContext->OMSetRenderTargets(1, &cubeMapper.cm_rtv[i], cubeMapper.cm_depthStencilView);

		_shaders[0].SetShaderParameters(_deviceContext, *(models[0]), cubeMapper.cameras[i], cubeMapper.lens, _lights[0], cam.GetCameraMatrix().Translation(), dTime);
		models[0]->Draw(_deviceContext, _shaders[0]);
		_shaders[0].ReleaseShaderParameters(_deviceContext);
	}
//reflection cube map done


//shadow cube map begin
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
//shadow cube map end
*/
	_deviceContext->RSSetViewports(1, &_D3D->viewport);
	_D3D->SetBackBufferRenderTarget();
	_D3D->BeginScene(clearColour);

	if (drawUI) {
		_rekt->draw(_deviceContext, shaderHUD, offScreenTexture.srv);
	}

	for (auto model : models) {

		/*shaderShadow.SetShaderParameters(_deviceContext, *model, cam.GetViewMatrix(), _lightvm, cam.GetProjectionMatrix(),
			_lightpm, pLight, cam.GetCameraMatrix().Translation(), offScreenTexture.srv);
		model->Draw(_deviceContext, shaderShadow);
		shaderShadow.ReleaseShaderParameters(_deviceContext);*/

		_shaders[0].SetShaderParameters(_deviceContext, *model, cam.GetViewMatrix(), cam.GetProjectionMatrix(), _lights[0],
			cam.GetCameraMatrix().Translation(), dTime);
		model->Draw(_deviceContext, _shaders[0]);
		_shaders[0].ReleaseShaderParameters(_deviceContext);
		break;
	}

	/*
	shaderCM.SetShaderParameters(_deviceContext, *(_models[1]), cam.GetViewMatrix(), cam.GetProjectionMatrix(), _lights[0],
		cam.GetCameraMatrix().Translation(), dTime, cubeMapper.cm_srv);
	_models[1]->Draw(_deviceContext, shaderCM);
	shaderCM.ReleaseShaderParameters(_deviceContext);
	*/


	/*//project texture onto the scene
	SMatrix texView = DirectX::XMMatrixLookAtLH(SVec3(0.0f, 0.0f, -1.0f), SVec3(0.0f, 0.0f, 0.0f), SVec3::Up);
	shaderPT.SetShaderParameters(_deviceContext, *_models[0], cam.GetViewMatrix(), cam.GetViewMatrix(), cam.GetProjectionMatrix(),
								cam.GetProjectionMatrix(), _lights[0], cam.GetCameraMatrix().Translation(), dTime, offScreenTexture.srv);
	models[1]->Draw(_deviceContext, shaderPT);
	shaderPT.ReleaseShaderParameters(_deviceContext);
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
#include "Renderer.h"
#include "SimpleMath.h"
#include "InputManager.h"


Renderer::Renderer(){
	_D3D = 0;
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



	mod.LoadModel(_device, "C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Models/terrainTex.fbx");
	mod.transform = mod.transform.CreateScale(SVec3(0.3f, 0.3f, 0.3f));

	_models.push_back(&mod);

	///Dragon/Dragon 2o5_fbx.fbx
	mod2.LoadModel(_device, "C:/Users/Senpai/Documents/Visual Studio 2015/Projects/Lab 5 lighting/Engine/Models/ball.fbx");	
	Math::Translate(mod2.transform, SVec3(0.0f, 10.0f, -10.0f));
	_models.push_back(&mod2);


	LightData ld(SVec3(1.0f), .1f, SVec3(1.0f), .3f, SVec3(1.0f), 0.5f);
	
	DirectionalLight light(ld, SVec4(0.0f, 0.0f, 1.0f, 1.0f));	//SVec4(0.0f, .707f, .707f, 1.f)
	_lights.push_back(light);

	pLight = PointLight(ld, SVec4(0.0f, 70.0f, 0.0f, 1.0f));

	SMatrix cMat;
	//cMat = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&eyePos), SVec3(0.0f, 0.0f, 0.0f), SVec3::Up);

	SMatrix projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(_D3D->_fieldOfView, _D3D->_screenAspect, SCREEN_NEAR, SCREEN_DEPTH);

	Camera cam(cMat, projectionMatrix);

	_controllers.push_back(Controller(&inMan));

	cam._controller = &_controllers[0];

	_cameras.push_back(cam);

	_rekt = new Rekt(_device, _deviceContext);
	screenRect = _rekt->AddUINODE(_rekt->getRoot(), SVec2(0.75f, 0.75f), SVec2(0.25f, 0.25f));
	offScreenTexture.Init(_device, 400u, 300u);

	return true;
}



bool Renderer::Frame(){

	for (Camera& c : _cameras) {
		c.update(0.016f);
	}

	//_models[1]->transform *= SMatrix::CreateFromAxisAngle(SVec3::Up, 0.01f);

	return RenderFrame(_models, _cameras[0]);
}


bool Renderer::RenderFrame(const std::vector<Model*>& models, const Camera& cam){


	
	D3D11_VIEWPORT altViewport;
	altViewport.Width = 400.0f;
	altViewport.Height = 300.0f;
	altViewport.MinDepth = 0.0f;
	altViewport.MaxDepth = 1.0f;
	altViewport.TopLeftX = 0.0f;
	altViewport.TopLeftY = 0.0f;
	
	SMatrix lightView = DirectX::XMMatrixLookAtLH(SVec3(pLight.pos.x, pLight.pos.y, pLight.pos.z), SVec3(0.0f, 0.0f, 0.0f), SVec3::Forward);

	//switch to drawing on ost for the prepass	//offScreenTexture.SetRenderTarget(_deviceContext, _D3D->GetDepthStencilView());
	_deviceContext->OMSetRenderTargets(1, &(offScreenTexture.rtv), _D3D->GetDepthStencilView());

	//then clear it, both the colours and the depth-stencil buffer
	_deviceContext->ClearRenderTargetView(offScreenTexture.rtv, ccb);
	_deviceContext->ClearDepthStencilView(_D3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


//to the small viewport
	_deviceContext->RSSetViewports(1, &altViewport);
	 //draw all you want to...


	
	for (auto model : models) {
		shaderDepth.SetShaderParameters(_deviceContext, *model, lightView, cam.GetProjectionMatrix());
		model->Draw(_deviceContext, shaderDepth);

		//shaderShadow.SetShaderParameters(_deviceContext, *model, lightView, lightView, cam.GetProjectionMatrix(),
		//	cam.GetProjectionMatrix(), pLight, cam.GetCameraMatrix().Translation(), offScreenTexture.srv);
		//model->Draw(_deviceContext, shaderShadow);
		//shaderShadow.ReleaseShaderParameters(_deviceContext);
	}
//back to the big viewport

	_deviceContext->RSSetViewports(1, &_D3D->viewport);

	//and back to the back buffer we go		//_D3D->SetBackBufferRenderTarget();
	_deviceContext->OMSetRenderTargets(1, &(_D3D->m_renderTargetView), _D3D->GetDepthStencilView());
	_D3D->SetBackBufferRenderTarget();


	_D3D->BeginScene(clearColour);	// Clear the buffers to begin the main pass
	_rekt->draw(_deviceContext, shaderHUD, offScreenTexture.srv);

	for (auto model : models) {

		/*
		//@TODO FIX FAKE TIMESTEP
		_shaders[0].SetShaderParameters(_deviceContext, *model, cam.GetViewMatrix(), cam.GetProjectionMatrix(), _lights[0], cam.GetCameraMatrix().Translation(), 0.016f);
		model->Draw(_deviceContext, _shaders[0]);
		_shaders[0].ReleaseShaderParameters(_deviceContext);
		break;
		*/

		shaderShadow.SetShaderParameters(_deviceContext, *model, cam.GetViewMatrix(), lightView, cam.GetProjectionMatrix(),
			cam.GetProjectionMatrix(), pLight, cam.GetCameraMatrix().Translation(), offScreenTexture.srv);
		model->Draw(_deviceContext, shaderShadow);
		shaderShadow.ReleaseShaderParameters(_deviceContext);

	}

	/*//project texture onto the scene
	SMatrix texView = DirectX::XMMatrixLookAtLH(SVec3(0.0f, 0.0f, -1.0f), SVec3(0.0f, 0.0f, 0.0f), SVec3::Up);
	shaderPT.SetShaderParameters(_deviceContext, *_models[0], cam.GetViewMatrix(), cam.GetViewMatrix(), cam.GetProjectionMatrix(),
								cam.GetProjectionMatrix(), _lights[0], cam.GetCameraMatrix().Translation(), 0.016f, offScreenTexture.srv);
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
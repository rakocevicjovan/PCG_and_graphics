#include "Renderer.h"
#include "InputManager.h"
#include "ResourceManager.h"



Renderer::Renderer() {}



Renderer::~Renderer() {}



#define EYE_POS _cam.GetCameraMatrix().Translation()


bool Renderer::Initialize(int windowWidth, int windowHeight, HWND hwnd, ResourceManager& resMan, D3D& d3d, Controller& ctrl)
{
	_d3d = &d3d;
	_resMan = &resMan;
	
	_device = d3d.GetDevice();
	_deviceContext = d3d.GetDeviceContext();

	_shMan.init(_device, hwnd);
	
	_rekt = new Rekt(_device, _deviceContext);
	screenRect = _rekt->AddUINODE(_rekt->getRoot(), SVec2(0.75f, 0.75f), SVec2(0.25f, 0.25f));

	// Setup the projection matrix.
	_fieldOfView = PI / 3.0f;
	_screenAspect = (float)windowWidth / (float)windowHeight;

	///CAMERA INITIALISATION - get this out of here, I want to support multiple cameras no reason to hardcode one like this
	_cam = Camera(SMatrix::Identity, DirectX::XMMatrixPerspectiveFovLH(_fieldOfView, _screenAspect, SCREEN_NEAR, SCREEN_DEPTH));
	_cam._controller = &ctrl;

	return true;
}



bool Renderer::Frame(float dTime, InputManager* inMan)
{
	elapsed += dTime;
	sinceInput += dTime;
	
	bool res = UpdateRenderContext(dTime);

	if (sinceInput < .33f)
		return true;

	if (inMan->IsKeyDown((short)'F'))
	{
		_cam._controller->toggleFly();
		sinceInput = 0;
	}

	return res;
}



void Renderer::setCameraMatrix(const SMatrix& camMatrix)
{
	_cam.SetCameraMatrix(camMatrix);
}



bool Renderer::UpdateRenderContext(float dTime)
{
	rc.cam = &_cam;
	rc.d3d = _d3d;
	rc.dTime = dTime;
	rc.elapsed = elapsed;
	rc.shMan = &_shMan;

	return true;
}









#pragma region oldScene

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


///RENDERING OLD TERRAIN
for (auto tm : _terrainModels) {
	shaderShadow.SetShaderParameters(_deviceContext, *tm, _cam.GetViewMatrix(), offScreenTexture._view, _cam.GetProjectionMatrix(),
		offScreenTexture._lens, pointLight, _cam.GetCameraMatrix().Translation(), offScreenTexture.srv);
	tm->Draw(_deviceContext, shaderShadow);
	shaderShadow.ReleaseShaderParameters(_deviceContext);
}

*/

#pragma endregion oldScene
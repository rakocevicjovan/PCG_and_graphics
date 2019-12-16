#include "Renderer.h"
#include "InputManager.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "Shader.h"


Renderer::Renderer()
{
	//sAlloc.init(sizeof(Renderable) * MAX_RENDERABLES);
	opaques.reserve(MAX_OPAQUES);
	transparents.reserve(MAX_TRANSPARENTS);
}



Renderer::~Renderer() {}



bool Renderer::Initialize(int windowWidth, int windowHeight, HWND hwnd, ResourceManager& resMan, D3D& d3d, Controller& ctrl)
{
	_d3d = &d3d;
	_resMan = &resMan;
	
	_device = d3d.GetDevice();
	_deviceContext = d3d.GetDeviceContext();

	_shMan.init(_device, hwnd);

	// Setup the projection matrix.
	_fieldOfView = PI / 3.0f;
	_screenAspect = (float)windowWidth / (float)windowHeight;

	_cam = Camera(SMatrix::Identity, DirectX::XMMatrixPerspectiveFovLH(_fieldOfView, _screenAspect, SCREEN_NEAR, SCREEN_DEPTH));
	_cam._controller = &ctrl;

	return true;
}



bool Renderer::Frame(float dTime, InputManager* inMan)
{
	elapsed += dTime;
	
	bool res = UpdateRenderContext(dTime);

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



void Renderer::SetOSTRenderTarget(OST& ost)
{
	ost.SetRenderTarget(_deviceContext);
}



void Renderer::RevertRenderTarget()
{
	_d3d->SetBackBufferRenderTarget();
}



void Renderer::RenderSkybox(const Camera& cam, Model& skybox, const CubeMapper& skyboxCubeMapper) 
{
	_d3d->setRSSolidNoCull();
	_d3d->SwitchDepthToLessEquals();
	_shMan.skyboxShader.SetShaderParameters(_deviceContext, cam, rc.dTime, skyboxCubeMapper.cm_srv);
	skybox.Draw(_deviceContext, _shMan.skyboxShader);
	rc.shMan->skyboxShader.ReleaseShaderParameters(_deviceContext);
	_d3d->SwitchDepthToDefault();
	_d3d->setRSSolidCull();
}



void Renderer::addToRenderQueue(const Renderable& renderable)
{
	if (renderable.mat->opaque)
		opaques.push_back(renderable);	//stack allocator could work, or just reserving... not sure really
	else
		transparents.push_back(renderable);
}



void Renderer::clearRenderQueue()
{
	opaques.clear();	//qKeys.clear();
	transparents.clear();
}



void Renderer::sortRenderQueue()
{
	std::sort(opaques.begin(), opaques.end());	//determine how to sort by overloading < for renderable
	std::sort(transparents.begin(), transparents.end());
}



void Renderer::flushRenderQueue()
{	
	for (const auto& r : opaques)
	{
		render(r);
	}

	for (const auto& r : transparents)
	{
		render(r);
	}
}



//mind all the pointers this can fail spectacularly if anything relocates...
void Renderer::render(const Renderable& r)
{
	//all these updates still have a cost, even if they were the same, so we must avoid that, assisted by sorting
	//2500 draws drop fps to ~30 fps when only setting the state once, to ~20 when every time even if same
	unsigned int stride = r.mat->stride;
	unsigned int offset = r.mat->offset;

	//update cbuffers
	r.mat->vertexShader->populateBuffers(_deviceContext, r.worldTransform, rc.cam->GetViewMatrix(), rc.cam->GetProjectionMatrix());
	r.mat->pixelShader->populateBuffers(_deviceContext, *r.pLight, rc.cam->GetPosition());

	//set shaders and similar geebees
	_deviceContext->IASetInputLayout(r.mat->vertexShader->_layout);
	_deviceContext->VSSetShader(r.mat->vertexShader->_vShader, NULL, 0);
	_deviceContext->PSSetShader(r.mat->pixelShader->_pShader, NULL, 0);
	_deviceContext->PSSetSamplers(0, 1, &r.mat->pixelShader->_sState);

	//extract to sort by, won't be very uniform... tex arrays can help though...
	for (int i = 0; i < r.mat->textures.size(); ++i)
		_deviceContext->PSSetShaderResources(r.mat->texturesAdded + i, 1, &(r.mat->textures[i]->srv));

	//extract to sort by... should be fairly uniform though
	_deviceContext->IASetPrimitiveTopology(r.mat->primitiveTopology);

	//these have to change each time unless I'm packing multiple meshes per buffer... can live with that tbh
	_deviceContext->IASetVertexBuffers(0, 1, &(r.mesh->_vertexBuffer), &stride, &offset);
	_deviceContext->IASetIndexBuffer(r.mesh->_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	_deviceContext->DrawIndexed(r.mesh->indexCount, 0, 0);
}











#pragma region yeOldeThings
/*
///PROJECT TEXTURE
SMatrix texView = DirectX::XMMatrixLookAtLH(SVec3(0.0f, 0.0f, -1.0f), SVec3(0.0f, 0.0f, 0.0f), SVec3::Up);
texProjector.SetShaderParameters(_deviceContext, modTerrain, cam.GetViewMatrix(), cam.GetViewMatrix(), cam.GetProjectionMatrix(),
							cam.GetProjectionMatrix(), _lights[0], cam.GetCameraMatrix().Translation(), dTime, offScreenTexture.baseSrv);
modTerrain.Draw(_deviceContext, texProjector);
texProjector.ReleaseShaderParameters(_deviceContext);

///RENDERING DEPTH TEXTURE
_deviceContext->RSSetViewports(1, &altViewport);	//to the shadow texture viewport
_deviceContext->OMSetRenderTargets(1, &(offScreenTexture.rtv), _D3D->GetDepthStencilView());	//switch to drawing on ost for the prepass	
_deviceContext->ClearRenderTargetView(offScreenTexture.rtv, ccb);	//then clear it, both the colours and the depth-stencil buffer
_deviceContext->ClearDepthStencilView(_D3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

for (auto tm : _terrainModels) {
	depth.SetShaderParameters(_deviceContext, *tm, offScreenTexture._view, offScreenTexture._lens);
	tm->Draw(_deviceContext, depth);
}
*/

#pragma endregion yeOldeThings
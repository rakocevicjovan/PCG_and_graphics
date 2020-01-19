#include "Renderer.h"
#include "InputManager.h"
#include "GameObject.h"
#include "Shader.h"


Renderer::Renderer()
{
}



Renderer::~Renderer() {}



bool Renderer::initialize(int windowWidth, int windowHeight, HWND hwnd, D3D& d3d)
{
	_d3d = &d3d;
	
	_device = d3d.GetDevice();
	_deviceContext = d3d.GetDeviceContext();

	_shMan.init(_device, hwnd);

	// Setup the projection matrix.
	_fieldOfView = PI / 3.0f;
	_aspectRatio = (float)windowWidth / (float)windowHeight;

	_cam = Camera(SMatrix::Identity, DirectX::XMMatrixPerspectiveFovLH(_fieldOfView, _aspectRatio, SCREEN_NEAR, SCREEN_DEPTH));

	createGlobalBuffers();

	return true;
}



bool Renderer::createGlobalBuffers()
{
	D3D11_BUFFER_DESC perCamBufferDesc = ShaderCompiler::createBufferDesc(sizeof(PerCameraBuffer));
	if (FAILED(_device->CreateBuffer(&perCamBufferDesc, NULL, &_perCamBuffer)))
		return false;

	D3D11_BUFFER_DESC perFrameBufferDesc = ShaderCompiler::createBufferDesc(sizeof(PerFrameBuffer));
	if (FAILED(_device->CreateBuffer(&perFrameBufferDesc, NULL, &_perFrameBuffer)))
		return false;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	PerCameraBuffer* dataPtr;

	SMatrix proj = _cam.GetProjectionMatrix();
	SMatrix pT = proj.Transpose();

	if (FAILED(_deviceContext->Map(_perCamBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr = (PerCameraBuffer*)mappedResource.pData;
	dataPtr->proj = pT;
	_deviceContext->Unmap(_perCamBuffer, 0);

	_deviceContext->VSSetConstantBuffers(10, 1, &_perCamBuffer);
	_deviceContext->VSSetConstantBuffers(11, 1, &_perFrameBuffer);

	return true;
}



bool Renderer::frame(float dTime)
{
	elapsed += dTime;
	
	updateRenderContext(dTime);
	_cam.Update(dTime);

	bool res = updatePerFrameBuffer(dTime);

	return res;
}



bool Renderer::updatePerFrameBuffer(float dTime)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	PerFrameBuffer* dataPtr;

	SMatrix view = _cam.GetViewMatrix();
	SMatrix vT = view.Transpose();

	if (FAILED(_deviceContext->Map(_perFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr = (PerFrameBuffer*)mappedResource.pData;
	dataPtr->mat = vT;
	dataPtr->delta = dTime;
	dataPtr->elapsed = elapsed;
	dataPtr->padding = SVec2(0.f);
	_deviceContext->Unmap(_perFrameBuffer, 0);

	return true;
}



void Renderer::setCameraMatrix(const SMatrix& camMatrix)
{
	_cam.SetCameraMatrix(camMatrix);
}



void Renderer::updateRenderContext(float dTime)
{
	rc.cam = &_cam;
	rc.d3d = _d3d;
	rc.dTime = dTime;
	rc.elapsed = elapsed;
	rc.shMan = &_shMan;
}



void Renderer::setOSTRenderTarget(OST& ost)
{
	ost.SetRenderTarget(_deviceContext);
}



void Renderer::setDefaultRenderTarget()
{
	_d3d->SetBackBufferRenderTarget();
}



void Renderer::renderSkybox(const Camera& cam, Model& skybox, const CubeMapper& skyboxCubeMapper) 
{
	_d3d->setRSSolidNoCull();
	_d3d->SwitchDepthToLessEquals();
	_shMan.skyboxShader.SetShaderParameters(_deviceContext, cam, rc.dTime, skyboxCubeMapper.cm_srv);
	skybox.Draw(_deviceContext, _shMan.skyboxShader);
	rc.shMan->skyboxShader.ReleaseShaderParameters(_deviceContext);
	_d3d->SwitchDepthToDefault();
	_d3d->setRSSolidCull();
}



void Renderer::flushRenderQueue()
{	
	for (const auto& r : _rQue.opaques)
		render(r);

	for (const auto& r : _rQue.transparents)
		render(r);
}



//mind all the pointers, this can fail spectacularly if anything relocates...
void Renderer::render(const Renderable& r) const
{
	unsigned int stride = r.mat->stride;
	unsigned int offset = r.mat->offset;

	//set cbuffers
	r.updateBuffersAuto(_deviceContext);
	r.setBuffers(_deviceContext);

	//set shaders and similar geebees
	_deviceContext->IASetInputLayout(r.mat->getVS()->_layout);
	_deviceContext->VSSetShader(r.mat->getVS()->_vShader, NULL, 0);
	_deviceContext->PSSetShader(r.mat->getPS()->_pShader, NULL, 0);
	_deviceContext->PSSetSamplers(0, 1, &r.mat->getPS()->_sState);

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
#include "Renderer.h"
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

	// Setup the projection matrix.
	_fieldOfView = PI / 3.0f;
	_aspectRatio = (float)windowWidth / (float)windowHeight;

	_cam = Camera(SMatrix::Identity, DirectX::XMMatrixPerspectiveFovLH(_fieldOfView, _aspectRatio, SCREEN_NEAR, SCREEN_DEPTH));

	return createGlobalBuffers();
}



bool Renderer::createGlobalBuffers()
{
	D3D11_BUFFER_DESC perCamBufferDesc = ShaderCompiler::createBufferDesc(sizeof(VSPerCameraBuffer));
	if (FAILED(_device->CreateBuffer(&perCamBufferDesc, NULL, &VS_perCamBuffer)))
		return false;

	SMatrix pT = _cam.GetProjectionMatrix().Transpose();

	CBuffer::updateWholeBuffer(_deviceContext, VS_perCamBuffer, &pT, sizeof(VSPerCameraBuffer));

	_deviceContext->VSSetConstantBuffers(VS_PER_CAMERA_CBUFFER_REGISTER, 1, &VS_perCamBuffer);


	D3D11_BUFFER_DESC perFrameBufferDesc = ShaderCompiler::createBufferDesc(sizeof(VSPerFrameBuffer));
	if (FAILED(_device->CreateBuffer(&perFrameBufferDesc, NULL, &VS_perFrameBuffer)))
		return false;
	_deviceContext->VSSetConstantBuffers(VS_PER_FRAME_CBUFFER_REGISTER, 1, &VS_perFrameBuffer);


	D3D11_BUFFER_DESC PS_perFrameBufferDesc = ShaderCompiler::createBufferDesc(sizeof(SVec4));
	if (FAILED(_device->CreateBuffer(&PS_perFrameBufferDesc, NULL, &PS_perFrameBuffer)))
		return false;
	_deviceContext->PSSetConstantBuffers(PS_PER_FRAME_CBUFFER_REGISTER, NULL, &PS_perFrameBuffer);

	return true;
}



bool Renderer::frame(float dTime)
{
	_elapsed += dTime;
	
	_cam.Update(dTime);

	updateRenderContext(dTime);

	return updatePerFrameBuffer(dTime);
}



bool Renderer::updatePerFrameBuffer(float dTime)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	
	VSPerFrameBuffer* vsFrameBufferPtr;
	SMatrix vT = _cam.GetViewMatrix().Transpose();

	if (FAILED(_deviceContext->Map(VS_perFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	vsFrameBufferPtr = reinterpret_cast<VSPerFrameBuffer*>(mappedResource.pData);
	vsFrameBufferPtr->viewMat = vT;
	vsFrameBufferPtr->delta = dTime;
	vsFrameBufferPtr->elapsed = _elapsed;
	_deviceContext->Unmap(VS_perFrameBuffer, 0);


	PSPerFrameBuffer* psFrameBufferPtr;
	if (FAILED(_deviceContext->Map(PS_perFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	psFrameBufferPtr = reinterpret_cast<PSPerFrameBuffer*>(mappedResource.pData);
	psFrameBufferPtr->elapsed = _elapsed;
	psFrameBufferPtr->delta = dTime;
	_deviceContext->Unmap(PS_perFrameBuffer, 0);

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
	rc.elapsed = _elapsed;
}



void Renderer::setOSTRenderTarget(OST& ost)
{
	ost.SetRenderTarget(_deviceContext);
}



void Renderer::setDefaultRenderTarget()
{
	_d3d->SetBackBufferRenderTarget();
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
	//update and set cbuffers
	r.updateBuffersAuto(_deviceContext);
	r.setBuffers(_deviceContext);

	//set shaders and similar geebees
	_deviceContext->VSSetShader(r.mat->getVS()->_vShader, NULL, 0);
	_deviceContext->PSSetShader(r.mat->getPS()->_pShader, NULL, 0);
	_deviceContext->IASetInputLayout(r.mat->getVS()->_layout);
	_deviceContext->PSSetSamplers(0, 1, &r.mat->getPS()->_sState);

	r.mat->bindTextures(_deviceContext);

	//could sort by this as well... should be fairly uniform though
	_deviceContext->IASetPrimitiveTopology(r.mat->primitiveTopology);

	//these have to change each time unless I'm packing multiple meshes per buffer... can live with that tbh

	unsigned int stride = r.mesh->_vertexBuffer._stride;
	unsigned int offset = r.mesh->_vertexBuffer._offset;

	_deviceContext->IASetVertexBuffers(0, 1, r.mesh->_vertexBuffer.ptr(), &stride, &offset);
	_deviceContext->IASetIndexBuffer(r.mesh->_indexBuffer.ptr(), DXGI_FORMAT_R32_UINT, 0);

	_deviceContext->DrawIndexed(r.mesh->indexCount, 0, 0);
}
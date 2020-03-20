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

	_cam = Camera(SMatrix::Identity, _fieldOfView, _aspectRatio, NEAR_PLANE, FAR_PLANE);

	return createGlobalBuffers();
}



bool Renderer::createGlobalBuffers()
{
	D3D11_BUFFER_DESC perCamBufferDesc = ShaderCompiler::createBufferDesc(sizeof(VSPerCameraBuffer));
	if (FAILED(_device->CreateBuffer(&perCamBufferDesc, NULL, &_VSperCamBuffer)))
		return false;

	SMatrix pT = _cam.GetProjectionMatrix().Transpose();

	CBuffer::updateWholeBuffer(_deviceContext, _VSperCamBuffer, &pT, sizeof(VSPerCameraBuffer));

	_deviceContext->VSSetConstantBuffers(VS_PER_CAMERA_CBUFFER_REGISTER, 1, &_VSperCamBuffer);


	D3D11_BUFFER_DESC perFrameBufferDesc = ShaderCompiler::createBufferDesc(sizeof(VSPerFrameBuffer));
	if (FAILED(_device->CreateBuffer(&perFrameBufferDesc, NULL, &_VSperFrameBuffer)))
		return false;
	_deviceContext->VSSetConstantBuffers(VS_PER_FRAME_CBUFFER_REGISTER, 1, &_VSperFrameBuffer);


	D3D11_BUFFER_DESC PS_perFrameBufferDesc = ShaderCompiler::createBufferDesc(sizeof(PSPerFrameBuffer));
	if (FAILED(_device->CreateBuffer(&PS_perFrameBufferDesc, NULL, &_PSperFrameBuffer)))
		return false;
	_deviceContext->PSSetConstantBuffers(PS_PER_FRAME_CBUFFER_REGISTER, 1, &_PSperFrameBuffer);

	return true;
}



bool Renderer::frame(float dTime)
{
	_elapsed += dTime;
	
	_cam.Update(dTime);

	updateRenderContext(dTime);

	return updatePerFrameBuffers(dTime);
}



bool Renderer::updatePerFrameBuffers(float dTime)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	
	VSPerFrameBuffer* vsFrameBufferPtr;
	SMatrix vT = _cam.GetViewMatrix().Transpose();
	SVec4 eyePos = Math::fromVec3(_cam.GetPosition(), 1.);

	if (FAILED(_deviceContext->Map(_VSperFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	vsFrameBufferPtr = reinterpret_cast<VSPerFrameBuffer*>(mappedResource.pData);
	vsFrameBufferPtr->viewMat = vT;
	vsFrameBufferPtr->delta = dTime;
	vsFrameBufferPtr->elapsed = _elapsed;
	_deviceContext->Unmap(_VSperFrameBuffer, 0);


	PSPerFrameBuffer* psFrameBufferPtr;
	if (FAILED(_deviceContext->Map(_PSperFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	psFrameBufferPtr = reinterpret_cast<PSPerFrameBuffer*>(mappedResource.pData);
	psFrameBufferPtr->eyePos = eyePos;
	psFrameBufferPtr->elapsed = _elapsed;
	psFrameBufferPtr->delta = dTime;
	_deviceContext->Unmap(_PSperFrameBuffer, 0);

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
	ost.setAsRenderTarget(_deviceContext);
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



// Mind all the pointers, this can fail spectacularly if anything relocates...
void Renderer::render(const Renderable& r) const
{
	// Update and set cbuffers
	r.updateBuffersAuto(_deviceContext);
	r.setBuffers(_deviceContext);

	// Set shaders and similar geebees
	_deviceContext->VSSetShader(r.mat->getVS()->_vsPtr, NULL, 0);
	_deviceContext->PSSetShader(r.mat->getPS()->_psPtr, NULL, 0);
	_deviceContext->IASetInputLayout(r.mat->getVS()->_layout);
	_deviceContext->PSSetSamplers(0, 1, &r.mat->getPS()->_sState);

	r.mat->bindTextures(_deviceContext);

	// Could sort by this as well... should be fairly uniform though
	_deviceContext->IASetPrimitiveTopology(r.mat->primitiveTopology);

	// Packing vertex buffers together could be a good idea eventually
	UINT stride = r.mesh->getStride();
	UINT offset = r.mesh->getOffset();

	_deviceContext->IASetVertexBuffers(0, 1, r.mesh->_vertexBuffer.ptr(), &stride, &offset);
	_deviceContext->IASetIndexBuffer(r.mesh->_indexBuffer.ptr(), DXGI_FORMAT_R32_UINT, 0);

	_deviceContext->DrawIndexed(r.mesh->indexCount, 0, 0);
}
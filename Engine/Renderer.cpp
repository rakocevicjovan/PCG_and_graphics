#include "Renderer.h"
#include "Shader.h"


Renderer::Renderer()
{
}



Renderer::~Renderer()
{
	_deviceContext->Release();
	_device->Release();

	_VSperCamBuffer->Release();
	_VSperFrameBuffer->Release();
	_PSperCamBuffer->Release();
	_PSperFrameBuffer->Release();
}



bool Renderer::initialize(int windowWidth, int windowHeight, D3D& d3d)
{
	_d3d = &d3d;
	
	_device = d3d.GetDevice();
	_deviceContext = d3d.GetDeviceContext();

	// Setup the projection matrix.
	_windowWidth = windowWidth;
	_windowHeight = windowHeight;

	_aspectRatio = _windowWidth / _windowHeight;
	_fieldOfView = PI / 3.0f;
	
	_cam = Camera(SMatrix::Identity, _fieldOfView, _aspectRatio, NEAR_PLANE, FAR_PLANE);

	_clusterManager = std::make_unique<ClusterManager>(CLUSTER_GRID_DIMS, (1 << 16), _device);	//30 * 17 * 16 = 8160 nodes

	return createGlobalBuffers();
}



bool Renderer::createGlobalBuffers()
{
	CBuffer::createBuffer(_device, CBuffer::createDesc(sizeof(VSPerCameraBuffer)), _VSperCamBuffer);
	SMatrix pT = _cam.GetProjectionMatrix().Transpose();
	CBuffer::updateWholeBuffer(_deviceContext, _VSperCamBuffer, &pT, sizeof(VSPerCameraBuffer));
	_deviceContext->VSSetConstantBuffers(VS_PER_CAMERA_CBUFFER_REGISTER, 1, &_VSperCamBuffer);

	CBuffer::createBuffer(_device, CBuffer::createDesc(sizeof(VSPerFrameBuffer)), _VSperFrameBuffer);
	_deviceContext->VSSetConstantBuffers(VS_PER_FRAME_CBUFFER_REGISTER, 1, &_VSperFrameBuffer);

	CBuffer::createBuffer(_device, CBuffer::createDesc(sizeof(PSPerCameraBuffer)), _PSperCamBuffer);
	PSPerCameraBuffer pspcb{ _windowWidth, _windowHeight, _cam._frustum._zn, _cam._frustum._zf};
	CBuffer::updateWholeBuffer(_deviceContext, _PSperCamBuffer, &pspcb, sizeof(PSPerCameraBuffer));
	_deviceContext->PSSetConstantBuffers(PS_PER_CAMERA_CBUFFER_REGISTER, 1, &_PSperCamBuffer);

	CBuffer::createBuffer(_device, CBuffer::createDesc(sizeof(PSPerFrameBuffer)), _PSperFrameBuffer);
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
	CBuffer::updateWithStruct(_deviceContext, _VSperFrameBuffer, 
		VSPerFrameBuffer{ _cam.GetViewMatrix().Transpose(), dTime, _elapsed});

	CBuffer::updateWithStruct(_deviceContext, _PSperFrameBuffer, 
		PSPerFrameBuffer{ Math::fromVec3(_cam.GetPosition(), 1.), _elapsed, dTime });

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

	_deviceContext->DrawIndexed(r.mesh->_indexBuffer.getIdxCount(), 0, 0);
}
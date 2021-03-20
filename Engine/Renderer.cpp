#include "Renderer.h"
#include "Shader.h"


Renderer::Renderer()
{
}



Renderer::~Renderer()
{
	_deviceContext->Release();
	_device->Release();
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
	_VSperCamBuffer.init(_device, CBuffer::createDesc(sizeof(VSPerCameraBuffer)));
	SMatrix pT{ _cam.GetProjectionMatrix().Transpose() };
	_VSperCamBuffer.update(_deviceContext, &pT, sizeof(VSPerCameraBuffer));
	_VSperCamBuffer.bindToVS(_deviceContext, VS_PER_CAMERA_CBUFFER_REGISTER);

	_VSperFrameBuffer.init(_device, CBuffer::createDesc(sizeof(VSPerFrameBuffer)));
	_VSperFrameBuffer.bindToVS(_deviceContext, VS_PER_FRAME_CBUFFER_REGISTER);

	_PSperCamBuffer.init(_device, CBuffer::createDesc(sizeof(PSPerCameraBuffer)));
	_PSperCamBuffer.updateWithStruct(_deviceContext, PSPerCameraBuffer{ _cam.GetProjectionMatrix(), _windowWidth, _windowHeight, _cam._frustum._zn, _cam._frustum._zf });
	_PSperCamBuffer.bindToPS(_deviceContext, PS_PER_CAMERA_CBUFFER_REGISTER);

	_PSperFrameBuffer.init(_device, CBuffer::createDesc(sizeof(PSPerFrameBuffer)));
	_PSperFrameBuffer.bindToPS(_deviceContext, PS_PER_FRAME_CBUFFER_REGISTER);

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
	_VSperFrameBuffer.updateWithStruct(_deviceContext, VSPerFrameBuffer{ _cam.GetViewMatrix().Transpose(), dTime, _elapsed });
	_PSperFrameBuffer.updateWithStruct(_deviceContext, PSPerFrameBuffer{ Math::fromVec3(_cam.GetPosition(), 1.), _elapsed, dTime });
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



void Renderer::setDefaultRenderTarget()
{
	_d3d->SetBackBufferRenderTarget();
}



void Renderer::flushRenderQueue()
{	
	for (const auto& r : _rQue._renderables)
		render(r);
}



// Mind all the pointers, this can fail spectacularly if anything relocates...
void Renderer::render(const Renderable& r) const
{
	// Update and set cbuffers
	r.updateBuffersAuto(_deviceContext);
	r.setBuffers(_deviceContext);

	// Set shaders and similar geebees
	r.mat->bind(_deviceContext);

	// Could sort by this as well... should be fairly uniform though
	_deviceContext->IASetPrimitiveTopology(r.mat->_primitiveTopology);

	// Packing vertex buffers together could be a good idea eventually
	UINT stride = r.mesh->getStride();
	UINT offset = r.mesh->getOffset();

	_deviceContext->IASetVertexBuffers(0, 1, r.mesh->_vertexBuffer.ptr(), &stride, &offset);
	_deviceContext->IASetIndexBuffer(r.mesh->_indexBuffer.ptr(), DXGI_FORMAT_R32_UINT, 0);

	_deviceContext->DrawIndexed(r.mesh->_indexBuffer.getIdxCount(), 0, 0);
}
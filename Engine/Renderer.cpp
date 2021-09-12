#include "pch.h"
#include "Renderer.h"
#include "Shader.h"


Renderer::Renderer() = default;


Renderer::~Renderer()
{
	_deviceContext->Release();
	_device->Release();
}


bool Renderer::initialize(int windowWidth, int windowHeight, D3D& d3d)
{
	_d3d = &d3d;
	
	_device = d3d.getDevice();
	_deviceContext = d3d.getContext();

	// Setup the projection matrix.
	_windowWidth = windowWidth;
	_windowHeight = windowHeight;

	_aspectRatio = static_cast<float>(windowWidth) / windowHeight;
	_fieldOfView = PI / 3.0f;
	
	_cam = Camera(SMatrix::Identity, _fieldOfView, _aspectRatio, NEAR_PLANE, FAR_PLANE);

	_clusterManager = std::make_unique<ClusterManager>(CLUSTER_GRID_DIMS, (1 << 16), _device);	//30 * 17 * 16 = 8160 nodes

	return createGlobalBuffers();
}


void Renderer::resize(uint16_t width, uint16_t height)
{
	_windowWidth = width;
	_windowHeight = height;

	_aspectRatio = width / height;

	// Set render target size, viewport etc... see what needs to happen
	// Keep in mind that rendering resolution might not change! Not sure how this should be handled.
}


bool Renderer::createGlobalBuffers()
{
	_perCamBuffer.init(_device, CBuffer::createDesc(sizeof(PerCameraBuffer)));
	updatePerCamBuffer(_windowWidth, _windowHeight);

	_VSperFrameBuffer.init(_device, CBuffer::createDesc(sizeof(VSPerFrameBuffer)));
	_VSperFrameBuffer.bindToVS(_deviceContext, PER_FRAME_CBUFFER_REGISTER);

	_PSperFrameBuffer.init(_device, CBuffer::createDesc(sizeof(PSPerFrameBuffer)));
	_PSperFrameBuffer.bindToPS(_deviceContext, PER_FRAME_CBUFFER_REGISTER);

	return true;
}


void Renderer::frame(float dTime)
{
	_elapsed += dTime;
	
	_cam.update(dTime);

	updateRenderContext(dTime);

	updatePerFrameBuffers(dTime);
}


void Renderer::updatePerFrameBuffers(float dTime)
{	
	_VSperFrameBuffer.updateWithStruct(_deviceContext, VSPerFrameBuffer{ _cam.getViewMatrix().Transpose(), dTime, _elapsed, SVec2() });
	_PSperFrameBuffer.updateWithStruct(_deviceContext, PSPerFrameBuffer{ Math::fromVec3(_cam.getPosition(), 1.), dTime, _elapsed, SVec2() });
}


void Renderer::updatePerCamBuffer(float ww, float wh)
{
	_perCamBuffer.updateWithStruct(_deviceContext, PerCameraBuffer{ _cam.getProjectionMatrix().Transpose(), ww, wh, _cam._frustum._zn, _cam._frustum._zf });
	_perCamBuffer.bindToVS(_deviceContext, PER_CAMERA_CBUFFER_REGISTER);
	_perCamBuffer.bindToPS(_deviceContext, PER_CAMERA_CBUFFER_REGISTER);
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
	// r.updateBuffersAuto(_deviceContext);
	r.setBuffers(_deviceContext);

	// Set shaders and similar geebees
	r.mat->bind(_deviceContext);

	r.mesh->_vertexBuffer.bind(_deviceContext);
	r.mesh->_indexBuffer.bind(_deviceContext);

	_deviceContext->DrawIndexed(r.mesh->_indexBuffer.getIdxCount(), 0, 0);
}
#pragma once
#include "RenderTarget.h"
#include "Camera.h"
#include "Viewport.h"
#include "Layer.h"
#include "ReservedBuffers.h"
#include "RenderQueue.h"

class RenderStage
{
public:

	RenderStage() = default;


	RenderStage(ID3D11Device* device, Camera* camera, RenderTarget* renderTarget, Viewport* viewport) : 
		_cam(camera), 
		_renderTarget(renderTarget), 
		_viewport(viewport)
	{
		_perCamBuffer.initFromStruct<PerCameraBuffer>(device);

		_VSperFrameBuffer.initFromStruct<VSPerFrameBuffer>(device);
		_PSperFrameBuffer.initFromStruct<PSPerFrameBuffer>(device);
	}

	
	void update(ID3D11DeviceContext* context)
	{
	}


	void prepare(ID3D11DeviceContext* context, float dTime, float elapsed)
	{
		_VSperFrameBuffer.updateWithStruct(context, VSPerFrameBuffer{ _cam->getCameraMatrix().Transpose(), _cam->getViewMatrix().Transpose(), dTime, elapsed });
		_VSperFrameBuffer.bindToVS(context, PER_FRAME_CBUFFER_REGISTER);

		_PSperFrameBuffer.updateWithStruct(context, PSPerFrameBuffer{ _cam->getCameraMatrix().Transpose(), _cam->getViewMatrix().Transpose(), Math::fromVec3(_cam->getPosition(), 1.), dTime, elapsed });
		_PSperFrameBuffer.bindToPS(context, PER_FRAME_CBUFFER_REGISTER);

		const auto& [w, h] = _renderTarget->size();

		_perCamBuffer.updateWithStruct(context, 
			PerCameraBuffer{ _cam->getProjectionMatrix().Transpose(), _cam->getProjectionMatrix().Invert().Transpose(), w, h, _cam->_frustum._zn, _cam->_frustum._zf });

		_perCamBuffer.bindToVS(context, PER_CAMERA_CBUFFER_REGISTER);
		_perCamBuffer.bindToPS(context, PER_CAMERA_CBUFFER_REGISTER);

		_renderTarget->bind(context);
		_renderTarget->clear(context);
		_viewport->bind(context);
	}


	RenderQueue _rQue;

private:

	uint8_t _ID{0u};

	CBuffer _perCamBuffer;
	CBuffer _VSperFrameBuffer;
	CBuffer _PSperFrameBuffer;

	// These might need to be shared between multiple stages therefore it's a bad idea to force ownership.
	// Better understanding of ownership will hopefully arise through some usage...
	Camera* _cam{};					// Shadow stage will render using the light centered camera... Reflections from mirrored cam etc.
	RenderTarget* _renderTarget{};		// Shadow map, scene buffer, post processed stages, back buffer
	Viewport* _viewport{};
};
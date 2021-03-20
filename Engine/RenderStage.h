#pragma once
#include "RenderTarget.h"
#include "Camera.h"
#include "Viewport.h"
#include "Layer.h"
#include "ReservedBuffers.h"

class RenderStage
{
public:

	RenderStage(ID3D11Device* device, Camera* camera, RenderTarget* renderTarget, Viewport* viewport) : 
		_cam(camera), 
		_renderTarget(renderTarget), 
		_viewport(viewport)
	{
		_perCamBuffer.createFromStruct<PerCameraBuffer>(device);

		_VSperFrameBuffer.createFromStruct<VSPerFrameBuffer>(device);
		_PSperFrameBuffer.createFromStruct<PSPerFrameBuffer>(device);
	}

	
	void update(ID3D11DeviceContext* context)
	{
		
	}


	void bind(ID3D11DeviceContext* context, float dTime, float elapsed)
	{
		_VSperFrameBuffer.updateWithStruct(context, VSPerFrameBuffer{ _cam->GetViewMatrix().Transpose(), dTime, elapsed });
		_PSperFrameBuffer.updateWithStruct(context, PSPerFrameBuffer{ Math::fromVec3(_cam->GetPosition(), 1.), dTime, elapsed });

		_perCamBuffer.bindToPS(context, PER_CAMERA_CBUFFER_REGISTER);

		_VSperFrameBuffer.bindToPS(context, PER_FRAME_CBUFFER_REGISTER);
		_PSperFrameBuffer.bindToPS(context, PER_FRAME_CBUFFER_REGISTER);

		_renderTarget->bind(context);
		_viewport->bind(context);
	}

private:

	uint8_t _ID{0u};

	CBuffer _perCamBuffer;
	CBuffer _VSperFrameBuffer;
	CBuffer _PSperFrameBuffer;

	// These might need to be shared between multiple stages therefore it's a bad idea to force ownership.
	// Better understanding of ownership will hopefully arise through some usage...
	Camera* _cam;					// Shadow stage will render using the light centered camera... Reflections from mirrored cam etc.
	RenderTarget* _renderTarget;		// Shadow map, scene buffer, post processed stages, back buffer
	Viewport* _viewport;
};
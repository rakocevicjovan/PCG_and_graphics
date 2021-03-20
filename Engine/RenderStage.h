#pragma once
#include "RenderTarget.h"
#include "Camera.h"
#include "Viewport.h"
#include "Layer.h"

class RenderStage
{
public:

	RenderStage(ID3D11Device* device)
	{
	}

	void bind(ID3D11DeviceContext* context)
	{
		context->PSSetShader
		_renderTarget.bind(context);
		_viewport.bind(context);
	}

private:

	uint8_t _ID{0u};

	Camera _cam;					// Shadow stage will render using the light centered camera... Reflections from mirrored cam etc.
	
	
	RenderTarget _renderTarget;		// Shadow map, scene buffer, post processed stages, back buffer
	Viewport _viewport;
	std::vector<Layer> _layers;
};
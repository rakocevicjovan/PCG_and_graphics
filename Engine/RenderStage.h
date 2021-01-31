#pragma once
#include "RenderTarget.h"
#include "Camera.h"


class RenderStage
{
public:

	RenderStage(ID3D11Device* device)
	{
		
	}

	void bind(ID3D11DeviceContext* context)
	{
		_renderTarget.bind(context);
	}

private:

	uint8_t _ID;

	RenderTarget _renderTarget;		// Shadow map, scene buffer, post processed stages, back buffer
	Camera _cam;					// Shadow stage will render using the light centered camera... Reflections from mirrored cam etc.
};
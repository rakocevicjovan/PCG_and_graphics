#pragma once
#include <d3d11_4.h>
#include <d3d11_4.h>


class Viewport
{
public:
	// This is too simple to required encapsulation (an opinion that will certainly bite me later)
	D3D11_VIEWPORT _viewport{};

	Viewport() {}

	Viewport(float w, float h, float minD = 0.f, float maxD = 1.f, float tlX = 0.f, float tlY = 0.f) 
		: _viewport(CreateViewport(w, h, minD, maxD, tlX, tlY)) {}

	void bind(ID3D11DeviceContext* context)
	{
		context->RSSetViewports(1, &_viewport);
	}

	static D3D11_VIEWPORT CreateViewport(float w, float h, float minD = 0.f, float maxD = 1.f, float tlX = 0.f, float tlY = 0.f)
	{
		return D3D11_VIEWPORT{ tlX, tlY, w, h, minD, maxD };
	}
};
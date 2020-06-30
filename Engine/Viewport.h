#pragma once
#include <d3d11.h>
#include <d3d11_4.h>


class Viewport
{
public:

	static D3D11_VIEWPORT createViewport(float w, float h, float minD = 0.f, float maxD = 1.f, float tlX = 0.f, float tlY = 0.f)
	{
		return D3D11_VIEWPORT{tlX, tlY, w, h, minD, maxD};
	}
};
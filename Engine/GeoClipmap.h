#pragma once
#include <d3d11_4.h>
#include "VBuffer.h"
#include "IBuffer.h"
#include "Texture.h"



class GeoClipmap
{
private:
	UINT _numLayers, _w, _x, _gapSize;

	// Texture arrays of _numLayers or separate textures per layer?
	Texture _heightMap;		// R32
	Texture _normalMap;		// RGBA 8bit

	struct GeoClipmapBuffer
	{
		SVec4 scaleTranslation;
		SVec4 hmapScaleTranslation;	// 1/(w, h) of texture,  xy origin of block in texture
		float zScale;
		float zTexScale;
		float alphaOffset;				// = ((n - 1) / 2 - w - 1); Precalc this
	};

public:

	GeoClipmap(UINT numLayers, UINT edgeVertexCount);

	void init(ID3D11Device* device);
	void update(ID3D11DeviceContext* context);
	void draw(ID3D11DeviceContext* context);
};
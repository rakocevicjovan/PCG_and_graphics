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
	Texture _elevation;		// R32
	Texture _normal;		// RGBA 8bit



public:

	GeoClipmap(UINT numLayers, UINT edgeVertexCount);

	void init(ID3D11Device* device);

	void draw();
};
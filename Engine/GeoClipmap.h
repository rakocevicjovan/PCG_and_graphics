#pragma once
#include <d3d11_4.h>
#include "VBuffer.h"
#include "IBuffer.h"
#include "Texture.h"
#include <array>



class GeoClipmap
{

	struct GeoClipmapBuffer
	{
		SVec4 scaleTranslation;
		SVec4 hmapScaleTranslation;	// 1/(w, h) of texture,  xy origin of block in texture
		float zScale;
		float zTexScale;
		float alphaOffset;				// = ((n - 1) / 2 - w - 1); Precalc this
		float transitionWidth;
	};

	struct RingLayer
	{
		// Cross transforms - multiplies the entire 4 * (m * 3) formation of cardinal blocks
		SVec2 _offset;				// From (0, 0) to bottom left
		SVec2 _size;				// From a unit square to the size of the hollow square
		SVec2 _blockSize;			// Size of each block - this is unifrom
		SVec2 _blockOffsets[12];	// Offsets for each block's bottom left vertex 
	};



private:

	UINT _numLayers, _edgeVertCount, _blockEdgeVertCount, _gapSize;
	float _coreVertSpacing;
	SVec2 _coreOffset;
	SVec2 _coreSize;
	std::vector<RingLayer> _layers;

	// Texture arrays of _numLayers or separate textures per layer?
	Texture _heightMap;		// R32
	Texture _normalMap;		// RGBA 8bit
	std::array<UINT, 2> _texSize;

public:

	GeoClipmap(UINT numLayers, UINT edgeSizeLog2, float xzScale);

	void init(ID3D11Device* device);
	void update(ID3D11DeviceContext* context);
	void draw(ID3D11DeviceContext* context);
};
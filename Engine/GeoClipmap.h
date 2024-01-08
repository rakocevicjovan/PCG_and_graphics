#pragma once
#include "VBuffer.h"
#include "IBuffer.h"
#include "Texture.h"
#include "Shader.h"

class TextureManager;

struct CellIndex
{
	int32_t x;
	int32_t y;

	bool operator==(const CellIndex& other) const
	{
		return x == other.x && y == other.y;
	}

	struct HashFunction
	{
		size_t operator()(const CellIndex& cellIndex) const
		{
			size_t rowHash = std::hash<int>()(cellIndex.x);
			size_t colHash = std::hash<int>()(cellIndex.y) << 1;
			return rowHash ^ colHash;
		}
	};
};

class GeoClipmap
{
private:

	struct GeoClipmapBuffer
	{
		SVec4 scaleTranslation;
		/*
		SVec4 hmapScaleTranslation;	// 1/(w, h) of texture,  xy origin of block in texture
		float zScale;
		float zTexScale;
		float alphaOffset;				// = ((n - 1) / 2 - w - 1); Precalc this
		float transitionWidth;
		*/
	};

	struct RingLayer
	{
		// Cross transforms - multiplies the entire 4 * (m * 3) formation of cardinal blocks
		SVec2 _offset;				// From (0, 0) to bottom left
		SVec2 _size;				// From a unit square to the size of the hollow square
		SVec2 _blockSize;			// Size of each block - this is unifrom
		SVec2 _vertexScale;			// Size scale for block vertex spacing
		SVec2 _blockOffsets[12];	// Offsets for each block's bottom left vertex 
	};

	struct GeoClipmapSource
	{
		std::string folderPath;
		uint32_t width;
		uint32_t height;
		float hRes = 1;
		float vRes = 1;
	};

	uint32_t _numLayers{};
	uint32_t _edgeVertCount{};
	uint32_t _blockEdgeVertCount{};
	uint32_t _texSize{};

	float _coreVertSpacing;
	SVec2 _coreOffset;
	SVec2 _coreSize;
	std::vector<RingLayer> _layers;

	std::pair<CellIndex, CellIndex> _prevVisibleRange;

	struct ActiveTexture
	{
		uint16_t sinceLastUse{ false };
		std::shared_ptr<Texture> texture;
	};

	std::unordered_map<CellIndex, ActiveTexture, CellIndex::HashFunction> _chunkTextures;

	// Texture arrays of _numLayers or separate textures per layer?
	Texture _heightMap;
	Texture _normalMap;

	VertexShader _vertShader;
	GeoClipmapBuffer _bufferData;
	CBuffer _cBuffer;

	VBuffer _coreVB;
	IBuffer _coreIB;

	VBuffer _blockVB;
	IBuffer _blockIB;

	VBuffer _crossVB;
	IBuffer _crossIB;

	VBuffer _rimVB;
	IBuffer _rimIB;

	VBuffer _degeneratesVB;
	IBuffer _degeneratesIB;

	void createBuffers(ID3D11Device* device);
	void createTextures(ID3D11Device* device);
	void createTransformData();
	void createGridVertices(UINT numCols, UINT numRows, std::vector<SVec2>& output);
	std::vector<UINT> createGridIndices(UINT numCols, UINT numRows);

public:

	GeoClipmap(UINT numLayers, UINT edgeSizeLog2, float xzScale);

	void init(ID3D11Device* device);

	void update(ID3D11DeviceContext* context, SVec3 cameraPosition, float farPlaneDistance);
	void draw(ID3D11DeviceContext* context);

	// Clipmap levels move in discrete steps
	SVec2 getLevelOffset(const SVec2& camPos, UINT level);

	TextureManager* _textureManager{};
};
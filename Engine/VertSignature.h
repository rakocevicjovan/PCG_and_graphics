#pragma once
#include <stdint.h>
#include <vector>
#include <d3d11.h>

enum class VAttribSemantic : uint8_t
{
	POS = 0u,
	TEX_COORD,
	NORMAL,
	TANGENT,
	BITANGENT,
	COL,
	B_IDX,
	B_WEIGHT,
	OTHER
};


enum class VAttribType : uint8_t
{
	FLOAT = 0u,
	FLOAT2,
	FLOAT3,
	FLOAT4,
	INT,
	INT2,
	INT3,
	INT4,
	UINT,
	UINT2,
	UINT3,
	UINT4,
	TYPELESS,
	TYPELESS1,
	TYPELESS2,
	TYPELESS3
};



struct VAttrib
{
	static const std::vector<uint16_t> VERT_TYPE_SIZE;

	VAttribSemantic _semantic;
	VAttribType _type;
	uint8_t _size;			// Needs to exist in order to support typeless... types...
	uint8_t _numElements;	// For multiple of same semantic type, say {TEX_COORD, float2, 4}

	VAttrib() {}

	VAttrib(VAttribSemantic s, VAttribType t, uint8_t numElements = 1u, uint8_t elemByteSize = 0u)
		: _semantic(s), _type(t), _numElements(numElements)
	{
		if (elemByteSize == 0u)
			elemByteSize = VERT_TYPE_SIZE[static_cast<size_t>(t)];

		_size = elemByteSize;
	}
};



struct VertSignature
{
	std::vector<VAttrib> _attributes;

	void addAttribute(VAttrib attrib);
	
	void addAttribute(VAttribSemantic s, VAttribType t, uint8_t numElements = 1u, uint8_t elemByteSize = 0u);

	uint16_t getVertByteWidth();

	uint16_t getOffsetOf(VAttribSemantic semantic, uint8_t index = 0u);

	std::vector<D3D11_INPUT_ELEMENT_DESC> createVertInLayElements();

	// This currently can't work with tex coordinates being U/UV/UVW
	inline UINT countAttribute(VAttribSemantic vertAttribSemantic) const
	{
		UINT result = 0u;
		for (const auto& vertAttrib : _attributes)
			if (vertAttrib._semantic == vertAttribSemantic)
				return vertAttrib._numElements;
		return result;
	}
};



static const std::vector<const char*> VERTSIG_TO_SEMANTIC
{
	"POSITION",
	"TEXCOORD",
	"NORMAL",
	"TANGENT",
	"BITANGENT",
	"COLOUR",
	"B_IDX",
	"B_WGT"
};


// Currently only uses 32 bits for everything, might want to reduce it
static const std::vector<DXGI_FORMAT> VERTSIG_TO_DXGIFORMAT
{
	DXGI_FORMAT_R32_FLOAT,
	DXGI_FORMAT_R32G32_FLOAT,
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32_SINT,
	DXGI_FORMAT_R32G32_SINT,
	DXGI_FORMAT_R32G32B32_SINT,
	DXGI_FORMAT_R32G32B32A32_SINT,
	DXGI_FORMAT_R32_UINT,
	DXGI_FORMAT_R32G32_UINT,
	DXGI_FORMAT_R32G32B32_UINT,
	DXGI_FORMAT_R32G32B32A32_UINT,
	DXGI_FORMAT_R32_TYPELESS,
	DXGI_FORMAT_R32G32_TYPELESS,
	DXGI_FORMAT_R32G32B32_TYPELESS,
	DXGI_FORMAT_R32G32B32A32_TYPELESS
};
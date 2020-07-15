#pragma once
#include <stdint.h>
#include <vector>
#include <d3d11_4.h>

enum class VAttribSemantic : uint8_t
{
	POS = 0u,
	TEX_COORD,
	NORMAL,
	TANGENT,
	BITANGENT,
	COL,
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
	TYPELESS
};



struct VAttrib
{
	static const std::vector<uint16_t> VERT_TYPE_SIZE;

	VAttribSemantic _semantic;
	VAttribType _type;
	uint8_t _size;			// Needs to exist in order to support typeless... types...
	uint8_t _numElements;	// For multiple of same semantic type, say {TEX_COORD, float2, 4}

	VAttrib() 
		: _semantic(VAttribSemantic::POS), _type(VAttribType::FLOAT3), _size(12u), _numElements(1u) {}

	VAttrib(VAttribSemantic s, VAttribType t, uint8_t size, uint8_t numElements)
		: _semantic(s), _type(t), _size(size), _numElements(numElements) {}

};



struct VertSignature
{
	std::vector<VAttrib> _attributes;

	uint16_t getVertByteWidth();

	uint16_t getOffsetOf(VAttribSemantic semantic, uint8_t index = 0u);
};



// THIS IS NOT INTENDED TO WORK FROM THIS CLASS, IT'S OUT OF SCOPE, BUT HANDY TO HAVE VISIBLE
/*
void createInLayDesc()
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout;

	for (int i = 0; i < _attributes.size(); ++i)
	{
		const auto& attr = _attributes[i];

		D3D11_INPUT_ELEMENT_DESC ied
		{
			"PLACEHOLDER",					// Get from static vector (uint is index, O(1) looup)
			0,								// Handle multiple elements with the same semantic (mat4)
			DXGI_FORMAT_R32G32B32_FLOAT, 	// Get from static vector
			0,								// Support multiple buffers at once, supported: [0, 15]
			D3D11_APPEND_ALIGNED_ELEMENT,	// Automagical...
			D3D11_INPUT_PER_VERTEX_DATA,	// Per vertex or per instance data
			0								// Advance
		};
	}
}
*/
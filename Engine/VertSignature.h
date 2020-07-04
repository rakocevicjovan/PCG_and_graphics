#pragma once
#include <stdint.h>
#include <vector>

enum class VAttribSemantic : uint8_t
{
	POS = 0u,
	TEX_COORD,
	NORMAL,
	TANGENT,
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
	VAttribSemantic _semantic;
	VAttribType _type;
	uint8_t _size;
	uint8_t _numElements;	// For multiple of same semantic type, say {TEX_COORD, float2, 4}

	VAttrib() 
		: _semantic(VAttribSemantic::POS), _type(VAttribType::FLOAT), _size(0u), _numElements(1u) { }

	VAttrib(VAttribSemantic s, VAttribType t, uint8_t size, uint8_t numElements)
		: _semantic(s), _type(t), _size(size), _numElements(numElements) {}
};



struct VertSignature
{
	std::vector<VAttrib> _attributes;

	uint8_t getVertByteWidth()
	{
		uint8_t total = 0u;

		for (auto attrib : _attributes)
			total += (attrib._size * attrib._numElements);

		return total;
	}
};
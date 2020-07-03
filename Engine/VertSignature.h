#pragma once
#include <stdint.h>
#include <vector>

enum class VAttribSemantic : uint8_t
{
	POS = 0u,
	TEX_COORD,
	NORMAL,
	TANGENT,
	COL
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
	UINT4
};


struct VAttrib
{
	VAttribSemantic _semantic;
	VAttribType _type;
	uint8_t _numElements;	// pos would be _type = float, _numElements = 3 for example
};



class VertSignature
{
	std::vector<VAttrib> _attributes;
};
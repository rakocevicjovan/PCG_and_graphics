#pragma once
#include <d3d11.h>
#include <vector>


enum class CBUFFER_FIELD_TYPE		{ BOOL, FLOAT, FLOAT4, MATRIX4 };
enum class CBUFFER_FIELD_CONTENT	{ TRANSFORM,  };

struct CBufferFieldDesc
{
	CBUFFER_FIELD_CONTENT _type;
	uint16_t _offset;
	uint16_t _size;

	CBufferFieldDesc(CBUFFER_FIELD_CONTENT type, uint16_t offset, uint16_t size)
		: _type(type), _offset(offset), _size(size)
	{}
};



struct CBufferMeta
{
	uint8_t _slot;
	size_t _size;
	std::vector<CBufferFieldDesc> _fields;
	
	CBufferMeta(uint8_t slot, const D3D11_BUFFER_DESC& desc) : _slot(slot), _size(desc.ByteWidth)
	{

	}
};


/*
class CBuffer
{
	ID3D11Buffer* buffer;
	CBufferMeta metadata;
};
*/
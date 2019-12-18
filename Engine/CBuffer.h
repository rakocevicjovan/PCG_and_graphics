#pragma once
#include <d3d11.h>
#include <vector>


enum class CBUFFER_FIELD_TYPE		{ BOOL, FLOAT, FLOAT4, MATRIX4 };
enum class CBUFFER_FIELD_CONTENT	{ TRANSFORM };

struct CBufferFieldDesc
{
	CBUFFER_FIELD_TYPE _type;
	CBUFFER_FIELD_CONTENT _content;
	uint16_t _offset;
	uint16_t _size;

	CBufferFieldDesc(CBUFFER_FIELD_CONTENT content, uint16_t offset, uint16_t size)
		: _content(content), _offset(offset), _size(size)
	{}
};



struct CBufferMeta
{
	uint8_t _slot;
	size_t _size;
	std::vector<CBufferFieldDesc> _fields;
	
	CBufferMeta(uint8_t slot, size_t size) : _slot(slot), _size(size)
	{

	}
};


class CBuffer
{
public:

	static bool map(ID3D11DeviceContext* cont, ID3D11Buffer*& cbuffer)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		if (FAILED(cont->Map(cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			return false;
	}

	static bool updateBuffer(ID3D11DeviceContext* cont, ID3D11Buffer*& cbuffer, UCHAR* data, size_t size, size_t offset)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		if (FAILED(cont->Map(cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			return false;

		UCHAR* dataPtr = (UCHAR*)mappedResource.pData;
		memcpy(dataPtr + offset, data, size);
		cont->Unmap(cbuffer, 0);
	}
};



/*
class CBuffer
{
	ID3D11Buffer* buffer;
	CBufferMeta metadata;
};
*/
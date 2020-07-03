#pragma once
#include <d3d11.h>
#include <vector>

// Basically halves the class size with the option to use 8 bit enums...
enum class CBUFFER_FIELD_TYPE : uint8_t		{ BOOL, FLOAT, FLOAT4, MATRIX4 };
enum class CBUFFER_FIELD_CONTENT : uint8_t { TRANSFORM, CSM };

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
	
	CBufferMeta() {}

	CBufferMeta(uint8_t slot, size_t size) : _slot(slot), _size(size) {}

	inline void addFieldDescription(CBUFFER_FIELD_CONTENT semantic, uint16_t offset, uint16_t size)
	{
		_fields.push_back(CBufferFieldDesc(semantic, offset, size));
	}
};



class CBuffer
{
public:

	ID3D11Buffer* _cbPtr;
	CBufferMeta _metaData;


	CBuffer() : _cbPtr(nullptr) {}

	CBuffer(const CBuffer& other) : _cbPtr(other._cbPtr)
	{
		if (_cbPtr)
			_cbPtr->AddRef();

		_metaData = other._metaData;
	}

	CBuffer& operator=(const CBuffer& other)
	{
		_cbPtr = other._cbPtr;
		_cbPtr->AddRef();
		_metaData = other._metaData;
		return *this;
	}

	~CBuffer()
	{
		if (_cbPtr)
			_cbPtr->Release();
	}



	CBuffer(ID3D11Device* device, const D3D11_BUFFER_DESC& desc)
	{
		if (!createBuffer(device, desc, _cbPtr))
		{
			OutputDebugStringA("Failed to create vertex buffer.");
			exit(1001);
		}
	}



	static inline bool createBuffer(ID3D11Device* device, const D3D11_BUFFER_DESC& desc, ID3D11Buffer*& buffer)
	{
		if (FAILED(device->CreateBuffer(&desc, NULL, &buffer)))
			return false;

		return true;
	}



	inline static D3D11_BUFFER_DESC createDesc(
		UINT byteWidth,
		D3D11_USAGE usage = D3D11_USAGE_DYNAMIC,
		D3D11_BIND_FLAG binding = D3D11_BIND_CONSTANT_BUFFER,
		D3D11_CPU_ACCESS_FLAG cpuAccessFlag = D3D11_CPU_ACCESS_WRITE,
		UINT miscFlag = 0u,
		UINT stride = 0u)
	{
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = byteWidth;
		cbDesc.Usage = usage;
		cbDesc.BindFlags = binding;
		cbDesc.CPUAccessFlags = cpuAccessFlag;
		cbDesc.MiscFlags = miscFlag;
		cbDesc.StructureByteStride = stride;
		return cbDesc;
	}



	inline static bool map(ID3D11DeviceContext* cont, ID3D11Buffer*& cbuffer, D3D11_MAPPED_SUBRESOURCE& mappedResource)
	{
		if (FAILED(cont->Map(cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			return false;

		return true;
	}



	inline static void updateField(ID3D11Buffer*& cbuffer, void* data, size_t size, size_t offset, D3D11_MAPPED_SUBRESOURCE mappedResource)
	{
		memcpy(static_cast<UCHAR*>(mappedResource.pData) + offset, data, size);
	}



	inline static void unmap(ID3D11DeviceContext* cont, ID3D11Buffer*& cbuffer)
	{
		cont->Unmap(cbuffer, 0);
	}



	// utility function for updating all fields at once (if the whole cbuffer CAN be set at once)
	static bool updateWholeBuffer(ID3D11DeviceContext* cont, ID3D11Buffer*& cbuffer, void* data, size_t size)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		
		if (!map(cont, cbuffer, mappedResource))
			return false;

		memcpy(mappedResource.pData, data, size);
		
		unmap(cont, cbuffer);

		return true;
	}


	template <typename PODStruct>
	static bool updateWithStruct(ID3D11DeviceContext* cont, ID3D11Buffer*& cbuffer, const PODStruct& s)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		if (!map(cont, cbuffer, mappedResource))
			return false;

		memcpy(mappedResource.pData, &s, sizeof(PODStruct));

		unmap(cont, cbuffer);

		return true;
	}
};
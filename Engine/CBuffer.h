#pragma once
#include <d3d11_4.h>
#include <wrl/client.h>
#include <vector>

// Basically halves the class size with the option to use 8 bit enums...
enum class CBUFFER_FIELD_TYPE : uint8_t		{ BOOL, FLOAT, FLOAT4, MATRIX4 };
enum class CBUFFER_FIELD_CONTENT : uint8_t { TRANSFORM, CSM };

struct CBufferFieldDesc
{
	CBUFFER_FIELD_TYPE _type{ CBUFFER_FIELD_TYPE::FLOAT4 };
	CBUFFER_FIELD_CONTENT _content;
	uint16_t _offset{0};
	uint16_t _size{16};

	CBufferFieldDesc(CBUFFER_FIELD_CONTENT content, uint16_t offset, uint16_t size)
		: _content(content), _offset(offset), _size(size)
	{}
};



struct CBufferMeta
{
	uint8_t _slot{ 0 };
	size_t _size{ 0 };
	std::vector<CBufferFieldDesc> _fields{};
	
	CBufferMeta() {}

	CBufferMeta(uint8_t slot, size_t size) : _slot(slot), _size(size) {}

	inline void addFieldDescription(CBUFFER_FIELD_CONTENT semantic, uint16_t offset, uint16_t size)
	{
		_fields.push_back(CBufferFieldDesc(semantic, offset, size));
	}
};



class CBuffer
{
private:

	Microsoft::WRL::ComPtr<ID3D11Buffer> _cbPtr{ nullptr };

public:

	CBuffer() {}

	CBuffer(ID3D11Device* device, const D3D11_BUFFER_DESC& desc)
	{
		init(device, desc);
	}

	// Very nice utility for less typing
	template <typename PODStruct>
	void createFromStruct(ID3D11Device* device)
	{
		init(device, CBuffer::createDesc(sizeof(PODStruct)));
	}

	void init(ID3D11Device* device, const D3D11_BUFFER_DESC& desc)
	{
		if (FAILED(device->CreateBuffer(&desc, NULL, _cbPtr.GetAddressOf())))
		{
			OutputDebugStringA("Failed to create vertex buffer.");
			exit(1001);
		}
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



	inline void map(ID3D11DeviceContext* cont, D3D11_MAPPED_SUBRESOURCE& mappedResource) const
	{
		if (FAILED(cont->Map(_cbPtr.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		{
			__debugbreak();
		}
	}


	inline void updateField(void* data, size_t size, size_t offset, D3D11_MAPPED_SUBRESOURCE& mappedResource) const
	{
		memcpy(static_cast<UCHAR*>(mappedResource.pData) + offset, data, size);
	}


	inline void unmap(ID3D11DeviceContext* cont) const
	{
		cont->Unmap(_cbPtr.Get(), 0);
	}


	// Utility function for updating all fields at once, which is the preferred option
	void update(ID3D11DeviceContext* cont, void* data, size_t size) const
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		
		map(cont, mappedResource);

		memcpy(mappedResource.pData, data, size);
		
		unmap(cont);
	}


	template <typename PODStruct>
	bool updateWithStruct(ID3D11DeviceContext* cont, const PODStruct& s)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		map(cont, mappedResource);

		memcpy(mappedResource.pData, &s, sizeof(PODStruct));

		unmap(cont);

		return true;
	}

	inline void bindToVS(ID3D11DeviceContext* context, uint8_t slot)
	{
		context->VSSetConstantBuffers(slot, 1, _cbPtr.GetAddressOf());
	}

	inline void bindToPS(ID3D11DeviceContext* context, uint8_t slot)
	{
		context->PSSetConstantBuffers(slot, 1, _cbPtr.GetAddressOf());
	}
};
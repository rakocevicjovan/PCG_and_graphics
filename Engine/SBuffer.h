#pragma once
#include <d3d11.h>


class SBuffer
{
	ID3D11Buffer* _sbPtr;

public:

	SBuffer::SBuffer() : _sbPtr(nullptr) {}



	SBuffer::SBuffer(ID3D11Device* device, UINT elementSize, UINT numElements)
	{
		_sbPtr = createSBuffer(device, elementSize, numElements);
	}



	static ID3D11Buffer* createSBuffer(ID3D11Device* device, UINT elementSize, UINT numElements)
	{
		ID3D11Buffer* structBuffer;

		D3D11_BUFFER_DESC sbDesc = {};
		sbDesc.ByteWidth = elementSize * numElements;
		sbDesc.Usage = D3D11_USAGE_DYNAMIC;
		sbDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		sbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		sbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		sbDesc.StructureByteStride = elementSize;

		HRESULT result = device->CreateBuffer(&sbDesc, NULL, &structBuffer);

		if (FAILED(result))
			return nullptr;

		return structBuffer;
	};



	static HRESULT createSBufferSRV(ID3D11Device* device, ID3D11Buffer* pBuffer, UINT elemWidth, UINT numElements, ID3D11ShaderResourceView*& ppSRVOut)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = 0u;
		desc.Buffer.ElementOffset = 0u;
		desc.Buffer.ElementWidth = elemWidth;
		desc.Buffer.NumElements = numElements;

		//D3D11_SRV_DIMENSION_BUFFEREX for raw buffers?
		//desc.BufferEx.FirstElement = 0;
		//desc.BufferEx.NumElements = numElements;

		return device->CreateShaderResourceView(pBuffer, &desc, &ppSRVOut);
	}



	/* https://docs.microsoft.com/en-us/windows/win32/direct3d12/typed-unordered-access-view-loads */
	static HRESULT createSBufferUAV(ID3D11Device* device, ID3D11Buffer* pBuffer, UINT numElements, ID3D11UnorderedAccessView*& sbuav)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};	ZeroMemory(&desc, sizeof(desc));

		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = 0;
		desc.Format = DXGI_FORMAT_UNKNOWN; // Must be DXGI_FORMAT_UNKNOWN for structured buffer views
		desc.Buffer.NumElements = numElements;

		return device->CreateUnorderedAccessView(pBuffer, &desc, &sbuav);
	}



	inline void upload(ID3D11DeviceContext* context, const void* data, UINT byteWidth)
	{
		D3D11_MAPPED_SUBRESOURCE resource;
		context->Map(_sbPtr, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		memcpy(resource.pData, data, byteWidth);
		context->Unmap(_sbPtr, 0);
	}



	inline ID3D11Buffer* getPtr() { return _sbPtr; }
};
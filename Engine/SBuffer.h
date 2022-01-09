#pragma once


class SBuffer
{
private:

	Microsoft::WRL::ComPtr<ID3D11Buffer> _sbPtr{};

public:

	SBuffer::SBuffer() = default;


	SBuffer::SBuffer(ID3D11Device* device, UINT elementSize, UINT numElements, UINT extraFlags = 0u)
	{
		_sbPtr = CreateSBuffer(device, elementSize, numElements, extraFlags);
	}


	static ID3D11Buffer* CreateSBuffer(ID3D11Device* device, UINT elementSize, UINT numElements, UINT extraFlags = 0u)
	{
		ID3D11Buffer* structBuffer;

		D3D11_BUFFER_DESC sbDesc{};
		sbDesc.ByteWidth = elementSize * numElements;
		sbDesc.Usage = D3D11_USAGE_DEFAULT;
		sbDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | extraFlags;
		sbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// Questionable
		sbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		sbDesc.StructureByteStride = elementSize;

		HRESULT result = device->CreateBuffer(&sbDesc, NULL, &structBuffer);

		assert(!FAILED(result));

		return structBuffer;
	};


	static HRESULT CreateSBufferSRV(ID3D11Device* device, ID3D11Buffer* pBuffer, UINT numElements, ID3D11ShaderResourceView*& ppSRVOut)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;	//D3D11_SRV_DIMENSION_BUFFEREX for raw buffers?
		
		// For structured buffers
		desc.Buffer.FirstElement = 0u;
		desc.Buffer.NumElements = numElements;
		
		// For non structured buffers - don't do both, elementWidth overwrites numelements since it's a union!
		//desc.Buffer.ElementOffset = 0u;
		//desc.Buffer.ElementWidth = x;

		return device->CreateShaderResourceView(pBuffer, &desc, &ppSRVOut);
	}


	/* https://docs.microsoft.com/en-us/windows/win32/direct3d12/typed-unordered-access-view-loads */
	static HRESULT createSBufferUAV(ID3D11Device* device, ID3D11Buffer* pBuffer, UINT numElements, ID3D11UnorderedAccessView*& sbuav)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc{};
		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = 0;
		desc.Buffer.NumElements = numElements;
		desc.Format = DXGI_FORMAT_UNKNOWN; // Must be DXGI_FORMAT_UNKNOWN for structured buffer views

		return device->CreateUnorderedAccessView(pBuffer, &desc, &sbuav);
	}


	inline void upload(ID3D11DeviceContext* context, const void* data, UINT byteWidth)
	{
		D3D11_MAPPED_SUBRESOURCE resource;
		context->Map(_sbPtr.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		memcpy(resource.pData, data, byteWidth);
		context->Unmap(_sbPtr.Get(), 0);
	}


	inline ID3D11Buffer* getPtr() { return _sbPtr.Get(); }
};
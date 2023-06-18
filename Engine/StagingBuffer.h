#pragma once

class StagingBuffer
{
private:

	Microsoft::WRL::ComPtr<ID3D11Buffer> _buffer{};
	uint32_t _size{0u};

public:


	StagingBuffer(ID3D11Device* device, uint32_t elementCount, uint32_t elementSize) : _size(elementCount * elementSize)
	{
		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = _size;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		desc.MiscFlags = 0;
		desc.StructureByteStride = elementSize;
		assert(!FAILED(device->CreateBuffer(&desc, NULL, _buffer.GetAddressOf())) && "Failed to create gpu buffer for updateable buffer.");
	}


	void update(ID3D11DeviceContext* context, void* data, uint32_t size)
	{
		auto buffer = _buffer.Get();
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		context->Map(_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(static_cast<UCHAR*>(mappedResource.pData), data, size);
		context->Unmap(_buffer.Get(), 0);
	}


	void propagateToGpuBuffer(ID3D11DeviceContext1* context, ID3D11Buffer* targetBuffer)
	{
		auto destBox = D3D11_BOX{ 0, 0, 0, _size, 0, 0 };

		// Subresources must be different, but may come from the same resource. 
		// Must have same type and compatible DXGI formats. 
		// Resources may not be mapped when this is called.
		context->CopySubresourceRegion1(
			targetBuffer, 0,
			0, 0, 0,
			_buffer.Get(), 0,
			&destBox,				// If nullptr is sent, the whole source resource is copied. CopyResource() can also be used in that case.
			0);						// D3D11_COPY_FLAGS, 0 allows partial overwrite and doesn't discard (I THINK, MUST CONFIRM!)
	}
};
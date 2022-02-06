#pragma once

// Untested code
// Represents a structured buffer on the GPU with D3D11_USAGE_DEFAULT and a staging buffer used to carry updates over to it
class UpdateableBuffer
{
private:

	Microsoft::WRL::ComPtr<ID3D11Buffer> _static{};


	// Eventually it would be better to structure the engine to have a pool of staging buffers, maybe raw byte buffers
	CBuffer _staging{};

public:

	UpdateableBuffer::UpdateableBuffer() = default;


	UpdateableBuffer::UpdateableBuffer(ID3D11Device* device, uint32_t gpuElementCount, uint32_t stagingElementCount, uint32_t elementSize)
	{
		assert((elementSize % 16 != 0) && "Element size must be a multiple of 16.");

		{
			D3D11_BUFFER_DESC desc{};
			desc.ByteWidth = gpuElementCount * elementSize;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			desc.StructureByteStride = elementSize;
			assert(!FAILED(device->CreateBuffer(&desc, NULL, _static.GetAddressOf())) && "Failed to create gpu buffer for updateable buffer.");
		}

		{
			_staging = CBuffer::CBuffer(device, CBuffer::createDesc(elementSize));
		}
	}


	void updateStaging(ID3D11DeviceContext* context, void* data, uint32_t size)
	{
		_staging.update(context, data, size);
	}


	void propagateToGpu(ID3D11DeviceContext1* context)
	{
		// Placeholder
		uint32_t updateSize{42};

		auto destBox = D3D11_BOX{ 0, 0, 0, updateSize, 0, 0 };

		// Subresources must be different, but may come from the same resource. 
		// Must have same type and compatible DXGI formats. 
		// Resources may not be mapped when this is called.
		context->CopySubresourceRegion1(
			_static.Get(), 0, 
			0, 0, 0,
			_staging.ptr(), 0, 
			&destBox,				// If nullptr is sent, the whole source resource is copied. CopyResource() can also be used in that case.
			0);						// D3D11_COPY_FLAGS, 0 allows partial overwrite and doesn't discard (I THINK, MUST CONFIRM!)
	}
};
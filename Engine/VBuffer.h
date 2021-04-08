#pragma once
#include "MeshDataStructs.h"
#include "VertSignature.h"
#include <d3d11_4.h>
#include <wrl\client.h>



class VBuffer
{
protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> _vbPtr{nullptr};

public:

	UINT _stride{0u};
	UINT _offset{0u};


	VBuffer() : _vbPtr(nullptr), _stride(0u), _offset(0u) {}


	VBuffer(ID3D11Device* device, std::vector<uint8_t>& vertices, VertSignature vs, UINT offset = 0u)
	{
		_stride = vs.getVertByteWidth();
		_offset = offset;

		D3D11_BUFFER_DESC vertexBufferDesc = createDesc(vertices.size());
		D3D11_SUBRESOURCE_DATA vertexData = createSubresourceData(vertices.data());
		createVertexBuffer(device, vertexBufferDesc, vertexData);
	}


	VBuffer(ID3D11Device* device, void* dataPtr, UINT byteWidth, UINT stride = 0u, UINT offset = 0u)
	{
		_stride = stride;
		_offset = offset;
		
		D3D11_BUFFER_DESC vertexBufferDesc = createDesc(byteWidth);
		D3D11_SUBRESOURCE_DATA vertexData = createSubresourceData(dataPtr);
		createVertexBuffer(device, vertexBufferDesc, vertexData);
	}


	// This will have to change when multiple buffers are at play
	inline void bind(ID3D11DeviceContext* context)
	{
		context->IASetVertexBuffers(0, 1, &_vbPtr, &_stride, &_offset);
	}



	inline D3D11_BUFFER_DESC createDesc(UINT size, D3D11_USAGE usage = D3D11_USAGE_IMMUTABLE)
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.ByteWidth = size;
		vertexBufferDesc.Usage = usage;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		return vertexBufferDesc;
	}

	inline ID3D11Buffer* const* ptr() const { return _vbPtr.GetAddressOf(); }
	inline ID3D11Buffer* ptrVar() { return _vbPtr.Get(); }

private:

	inline D3D11_SUBRESOURCE_DATA createSubresourceData(void* dataPtr)
	{
		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = dataPtr;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		return vertexData;
	}


	inline void createVertexBuffer(ID3D11Device* device, D3D11_BUFFER_DESC& vbDesc, D3D11_SUBRESOURCE_DATA& srData)
	{
		if (FAILED(device->CreateBuffer(&vbDesc, &srData, _vbPtr.GetAddressOf())))
		{
			OutputDebugStringA("Failed to create vertex buffer.");
			exit(1001);
		}
	}
};
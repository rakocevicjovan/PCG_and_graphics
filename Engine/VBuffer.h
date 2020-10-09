#pragma once
#include "MeshDataStructs.h"
#include "VertSignature.h"
#include <d3d11.h>



class VBuffer
{
protected:
	ID3D11Buffer* _vbPtr;

public:

	UINT _stride;
	UINT _offset;


	VBuffer() : _vbPtr(nullptr), _stride(0u), _offset(0u) {}

	VBuffer(const VBuffer& other) : _vbPtr(other._vbPtr), _stride(other._stride), _offset(other._offset)
	{
		if (_vbPtr)
			_vbPtr->AddRef();
	}

	VBuffer& operator=(const VBuffer& other)
	{
		_vbPtr = other._vbPtr;

		if(_vbPtr)
			_vbPtr->AddRef();

		_stride = other._stride;
		_offset = other._offset;
		return *this;
	}

	~VBuffer()
	{
		if (_vbPtr)
			_vbPtr->Release();
	}


	VBuffer(ID3D11Device* device, std::vector<uint8_t>& vertices, VertSignature vs, UINT offset = 0u)
	{
		_stride = vs.getVertByteWidth();
		_offset = offset;

		D3D11_BUFFER_DESC vertexBufferDesc = createDesc(vertices.size());
		D3D11_SUBRESOURCE_DATA vertexData = createSubresourceData(vertices.data());
		createVertexBuffer(device, vertexBufferDesc, vertexData, _vbPtr);
	}


	VBuffer(ID3D11Device* device, void* dataPtr, UINT byteWidth, UINT stride = 0u, UINT offset = 0u)
	{
		_stride = stride;
		_offset = offset;
		
		D3D11_BUFFER_DESC vertexBufferDesc = createDesc(byteWidth);
		D3D11_SUBRESOURCE_DATA vertexData = createSubresourceData(dataPtr);
		createVertexBuffer(device, vertexBufferDesc, vertexData, _vbPtr);
	}



	inline D3D11_BUFFER_DESC createDesc(UINT size, D3D11_USAGE usage = D3D11_USAGE_IMMUTABLE)
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.ByteWidth = size;
		vertexBufferDesc.Usage = usage;	// Faster than default?
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		return vertexBufferDesc;
	}


	inline D3D11_SUBRESOURCE_DATA createSubresourceData(void* dataPtr)
	{
		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = dataPtr;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		return vertexData;
	}


	inline void createVertexBuffer(ID3D11Device* device, D3D11_BUFFER_DESC& vbDesc, D3D11_SUBRESOURCE_DATA& srData, ID3D11Buffer*& vbPtr)
	{
		if (FAILED(device->CreateBuffer(&vbDesc, &srData, &vbPtr)))
		{
			OutputDebugStringA("Failed to create vertex buffer.");
			exit(1001);
		}
	}

	inline ID3D11Buffer* const * ptr() const { return &_vbPtr; }
	inline ID3D11Buffer*& ptrVar() { return _vbPtr; }
};
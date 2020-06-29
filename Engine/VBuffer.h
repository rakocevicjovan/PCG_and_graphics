#pragma once
#include "MeshDataStructs.h"

class VBuffer
{
protected:
	ID3D11Buffer* _vbPtr;

public:

	VBuffer() : _vbPtr(nullptr), _stride(0u), _offset(0u) {}

	~VBuffer()
	{
		if (_vbPtr)
			_vbPtr->Release();
	}



	template <typename VertexType>
	VBuffer(ID3D11Device* device, std::vector<VertexType>& vertices, UINT offset = 0u)
		: _stride(sizeof(VertexType)), _offset(offset)
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertices.size();
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = vertices.data();
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexData, &_vbPtr)))
		{
			OutputDebugStringA("Failed to create vertex buffer.");
			exit(1001);
		}
	}



	inline ID3D11Buffer* const * ptr() const { return &_vbPtr; }
	inline ID3D11Buffer*& ptrVar() { return _vbPtr; }

	UINT _stride;
	UINT _offset;
};
#pragma once
#include <vector>
#include <d3d11.h>



class IBuffer
{
protected:
	ID3D11Buffer* _ibPtr;
	UINT _count;

public:



	IBuffer() : _ibPtr(nullptr), _count(0u) {}



	~IBuffer()
	{
		//if (_ibPtr) _ibPtr->Release();
	}



	//template <typename IndexType>
	IBuffer(ID3D11Device* device, std::vector<UINT>& indices) 
		: _count(indices.size())
	{
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.ByteWidth = sizeof(UINT) * indices.size();
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexData;
		indexData.pSysMem = indices.data();
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		if (FAILED(device->CreateBuffer(&indexBufferDesc, &indexData, &_ibPtr)))
		{
			OutputDebugStringA("Failed to create index buffer.");
			exit(1001);
		}
	}

	inline ID3D11Buffer* ptr() const { return _ibPtr; }
	inline ID3D11Buffer*& ptrVar() { return _ibPtr; }

	inline UINT getIdxCount() { return _count; }
	inline void setIdxCount(UINT count) { _count = count; }
};
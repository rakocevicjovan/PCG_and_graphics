#pragma once


class IBuffer
{
protected:
	ID3D11Buffer* _indexBuffer;
	UINT _count;

public:

	IBuffer() : _indexBuffer(nullptr), _count(0u) {}

	template <typename IndexType>
	IBuffer(ID3D11Device* device, std::vector<IndexType>& indices) 
		: _count(indices.size())
	{
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.ByteWidth = sizeof(IndexType) * indices.size();
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexData;
		indexData.pSysMem = indices.data();
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		if (FAILED(device->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer.ptrVar())))
		{
			OutputDebugStringA("Failed to create index buffer.");
			exit(1001);
		}
	}

	inline ID3D11Buffer* ptr() const { return _indexBuffer; }
	inline ID3D11Buffer*& ptrVar() { return _indexBuffer; }

	inline UINT getIdxCount() { return _count; }
	inline void setIdxCount(UINT count) { _count = count; }
};
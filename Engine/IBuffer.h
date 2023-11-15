#pragma once



class IBuffer
{
protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> _ibPtr{nullptr};
	UINT _count{0u};

public:

	IBuffer() {}

	//template <typename IndexType>
	IBuffer(ID3D11Device* device, const std::vector<UINT>& indices) 
		: _count(indices.size())
	{
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.ByteWidth = sizeof(UINT) * indices.size();
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexData;
		indexData.pSysMem = indices.data();
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		if (FAILED(device->CreateBuffer(&indexBufferDesc, &indexData, _ibPtr.GetAddressOf())))
		{
			OutputDebugStringA("Failed to create index buffer.");
			exit(1001);
		}
	}

	inline void bind(ID3D11DeviceContext* context)
	{
		context->IASetIndexBuffer(_ibPtr.Get(), DXGI_FORMAT_R32_UINT, 0);
	}

	inline ID3D11Buffer* ptr() const { return _ibPtr.Get(); }

	inline UINT getIdxCount() { return _count; }
};
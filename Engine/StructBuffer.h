#pragma once
#include <d3d11.h>


class StructBuffer
{
public:
	static ID3D11Buffer* createStructBuffer(ID3D11Device* device, UINT elementSize, UINT numElements)
	{
		ID3D11Buffer* structBuffer;

		D3D11_BUFFER_DESC sbDesc = {};
		sbDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		sbDesc.ByteWidth = elementSize * numElements;
		sbDesc.Usage = D3D11_USAGE_DYNAMIC;
		sbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		sbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		sbDesc.StructureByteStride = elementSize;

		HRESULT result = device->CreateBuffer(&sbDesc, NULL, &structBuffer);

		if (FAILED(result))
			return nullptr;

		return structBuffer;
	};
};
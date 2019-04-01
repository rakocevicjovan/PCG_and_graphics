#pragma once
#include "InstancedShader.h"
#include "Model.h"



InstancedShader::InstancedShader() : ShaderBase()
{
}



InstancedShader::~InstancedShader()
{
	DECIMATE(_instanceBuffer);
}



bool InstancedShader::Initialize(ID3D11Device* device, HWND hwnd, const std::vector<std::wstring> filePaths, 
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc, unsigned int instanceBufferSizeInElements)
{
	ShaderBase::Initialize(device, hwnd, filePaths, layoutDesc, samplerDesc);

	D3D11_BUFFER_DESC instanceBufferDesc;
	instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	instanceBufferDesc.ByteWidth = sizeof(InstanceData) * instanceBufferSizeInElements;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	if (FAILED(device->CreateBuffer(&instanceBufferDesc, NULL, &_instanceBuffer)))
		return false;

	return true;
}



bool InstancedShader::SetShaderParameters(SPBase* spb)
{
	ShaderBase::SetShaderParameters(spb);

	ShaderParametersLight* spl = (ShaderParametersLight*)spb;
	
	InstanceData* instanceData;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(spl->deviceContext->Map(_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))	return false;
	memcpy(mappedResource.pData, _instanceData.data(), _instanceData.size() * sizeof(InstanceData));
	spl->deviceContext->Unmap(_instanceBuffer, 0);
}



bool InstancedShader::UpdateInstanceData(const std::vector<InstanceData>& instanceData)
{
	_instanceData = instanceData;
	_instanceCount = _instanceData.size();
	return false;
}

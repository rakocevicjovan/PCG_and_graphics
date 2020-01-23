#pragma once
#include "InstancedShader.h"
#include "Model.h"



InstancedShader::InstancedShader()
{
}



InstancedShader::~InstancedShader()
{
	DECIMATE(_instanceBuffer);
}



bool InstancedShader::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc, unsigned int instanceBufferSizeInElements)
{
	bool result = true;

	_filePaths = filePaths;

	result &= shc.compileVS(filePaths.at(0), layoutDesc, _vertexShader, _layout);
	result &= shc.compilePS(filePaths.at(1), _pixelShader);
	result &= shc.createSamplerState(samplerDesc, _sampleState);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC mmBuffDesc = ShaderCompiler::createBufferDesc(sizeof(WMBuffer));
	result &= shc.createConstantBuffer(mmBuffDesc, _matrixBuffer);

	D3D11_BUFFER_DESC lightBuffDesc = ShaderCompiler::createBufferDesc(sizeof(LightBuffer));
	result &= shc.createConstantBuffer(lightBuffDesc, _lightBuffer);


	D3D11_BUFFER_DESC instanceBufferDesc = 
		shc.createBufferDesc(
			sizeof(InstanceData) * instanceBufferSizeInElements, 
			D3D11_USAGE_DYNAMIC,
			D3D11_BIND_VERTEX_BUFFER,
			D3D11_CPU_ACCESS_WRITE
		);
	shc.createConstantBuffer(instanceBufferDesc, _instanceBuffer);
	
	return result;
}



bool InstancedShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& mMat, const Camera& cam, const PointLight& pLight, float dTime)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WMBuffer* wmptr;
	InstanceData* instanceDataPtr;
	LightBuffer* lightBufferPtr;

	SMatrix mT = mMat.Transpose();

	
	//WMBuffer
	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	wmptr = (WMBuffer*)mappedResource.pData;
	wmptr->world = mT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	//LightBuffer
	if (FAILED(deviceContext->Map(_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	lightBufferPtr = (LightBuffer*)mappedResource.pData;
	lightBufferPtr->alc = pLight.alc;
	lightBufferPtr->ali = pLight.ali;
	lightBufferPtr->dlc = pLight.dlc;
	lightBufferPtr->dli = pLight.dli;
	lightBufferPtr->slc = pLight.slc;
	lightBufferPtr->sli = pLight.sli;
	lightBufferPtr->pos = pLight.pos;
	lightBufferPtr->ePos = Math::fromVec3(cam.GetCameraMatrix().Translation(), 1.f);
	deviceContext->Unmap(_lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &_lightBuffer);


	if (FAILED(deviceContext->Map(_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	memcpy(mappedResource.pData, _instanceData.data(), _instanceData.size() * sizeof(InstanceData));
	deviceContext->Unmap(_instanceBuffer, 0);

	deviceContext->IASetInputLayout(_layout);
	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &_sampleState);


	return true;
}



void InstancedShader::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	//deviceContext->PSSetShaderResources(0, 1, &(_unbinder[0]));
}



bool InstancedShader::UpdateInstanceData(const std::vector<InstanceData>& instanceData)
{
	_instanceData = instanceData;	//std::move(instanceData);
	_instanceCount = _instanceData.size();
	return false;
}

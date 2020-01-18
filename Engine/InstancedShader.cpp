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



bool InstancedShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, Model& model, const Camera& cam, const PointLight& pLight, float dTime)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* matrixBufferPtr;
	InstanceData* instanceDataPtr;
	VariableBuffer* varBufferPtr;
	LightBuffer* lightBufferPtr;

	SMatrix mT = model.transform.Transpose();
	SMatrix vT = cam.GetViewMatrix().Transpose();
	SMatrix pT = cam.GetProjectionMatrix().Transpose();

	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	matrixBufferPtr = (MatrixBuffer*)mappedResource.pData;
	matrixBufferPtr->world = mT;
	matrixBufferPtr->view = vT;
	matrixBufferPtr->projection = pT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	if (FAILED(deviceContext->Map(_variableBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	varBufferPtr = (VariableBuffer*)mappedResource.pData;
	varBufferPtr->deltaTime = dTime;
	varBufferPtr->padding = SVec3();
	deviceContext->Unmap(_variableBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &_variableBuffer);


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
	deviceContext->PSSetShaderResources(0, 1, &(_unbinder[0]));
}



bool InstancedShader::UpdateInstanceData(const std::vector<InstanceData>& instanceData)
{
	_instanceData = instanceData;	//std::move(instanceData);
	_instanceCount = _instanceData.size();
	return false;
}

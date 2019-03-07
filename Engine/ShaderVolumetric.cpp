#include "ShaderVolumetric.h"
#include "Model.h"

ShaderVolumetric::ShaderVolumetric() : ShaderBase()
{
}


ShaderVolumetric::~ShaderVolumetric()
{
}


bool ShaderVolumetric::Initialize(ID3D11Device* device, HWND hwnd, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{
	this->filePaths = filePaths;
	if (!ShaderBase::Initialize(device, hwnd, filePaths, layoutDesc, samplerDesc))
		return false;

	D3D11_BUFFER_DESC light2desc;
	light2desc.Usage = D3D11_USAGE_DYNAMIC;
	light2desc.ByteWidth = sizeof(LightBuffer2);
	light2desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	light2desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	light2desc.MiscFlags = 0;
	light2desc.StructureByteStride = 0;
	if (FAILED(device->CreateBuffer(&light2desc, NULL, &_lightBuffer2)))
		return false;

	D3D11_BUFFER_DESC viewRayDesc;
	viewRayDesc.Usage = D3D11_USAGE_DYNAMIC;
	viewRayDesc.ByteWidth = sizeof(ViewRayBuffer);
	viewRayDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	viewRayDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	viewRayDesc.MiscFlags = 0;
	viewRayDesc.StructureByteStride = 0;

	if (FAILED(device->CreateBuffer(&viewRayDesc, NULL, &_viewRayBuffer)))
		return false;

	return true;
}

bool ShaderVolumetric::setLightData(ID3D11DeviceContext* dc, const PointLight& pLight)
{
	LightBuffer2* lightBufferPtr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	if (FAILED(dc->Map(_lightBuffer2, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	lightBufferPtr = (LightBuffer2*)mappedResource.pData;
	lightBufferPtr->alc = pLight.alc;
	lightBufferPtr->ali = pLight.ali;
	lightBufferPtr->dlc = pLight.dlc;
	lightBufferPtr->dli = pLight.dli;
	lightBufferPtr->slc = pLight.slc;
	lightBufferPtr->sli = pLight.sli;
	lightBufferPtr->pos = pLight.pos;

	dc->Unmap(_lightBuffer2, 0);
}



bool ShaderVolumetric::SetShaderParameters(ID3D11DeviceContext* deviceContext, Model& model, const Camera& camera, float elapsed)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* dataPtr;
	ViewRayBuffer* dataPtr2;
	VariableBuffer* dataPtr3;

	SMatrix mT = model.transform.Transpose();
	SMatrix vT = camera.GetViewMatrix().Transpose();
	SMatrix pT = camera.GetProjectionMatrix().Transpose();

	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr = (MatrixBuffer*)mappedResource.pData;	// Get a pointer to the data in the constant buffer.
	dataPtr->world = mT;
	dataPtr->view = vT;
	dataPtr->projection = pT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0u, 1, &_matrixBuffer);
	

	LightBuffer2* lightBufferPtr;

	if (FAILED(deviceContext->Map(_lightBuffer2, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	lightBufferPtr = (LightBuffer2*)mappedResource.pData;
	lightBufferPtr->alc = SVec3();
	lightBufferPtr->ali = 0;
	lightBufferPtr->dlc = SVec3();
	lightBufferPtr->dli = 0;
	lightBufferPtr->slc = SVec3();
	lightBufferPtr->sli = 0;
	lightBufferPtr->pos = SVec4();

	deviceContext->Unmap(_lightBuffer2, 0);
	deviceContext->PSSetConstantBuffers(0u, 1, &_lightBuffer2);


	if (FAILED(deviceContext->Map(_variableBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	dataPtr3 = (VariableBuffer*)mappedResource.pData;
	dataPtr3->deltaTime = elapsed;
	dataPtr3->padding = SVec3();
	deviceContext->Unmap(_variableBuffer, 0);
	deviceContext->PSSetConstantBuffers(1u, 1, &_variableBuffer);



	//view data - updates per frame
	if (FAILED(deviceContext->Map(_viewRayBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	dataPtr2 = (ViewRayBuffer*)mappedResource.pData;
	dataPtr2->ePos = Math::fromVec3(camera.GetCameraMatrix().Translation(), 1.0f);
	deviceContext->Unmap(_viewRayBuffer, 0);
	deviceContext->PSSetConstantBuffers(2u, 1, &_viewRayBuffer);

	deviceContext->IASetInputLayout(_layout);
	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &_sampleState);

	return true;
}
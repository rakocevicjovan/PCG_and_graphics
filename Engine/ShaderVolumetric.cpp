#include "ShaderVolumetric.h"
#include "Model.h"

ShaderVolumetric::ShaderVolumetric()
{
}


ShaderVolumetric::~ShaderVolumetric()
{
}


bool ShaderVolumetric::Initialize(ID3D11Device* device, HWND hwnd, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{
	/*
	this->_filePaths = filePaths;
	if (!ShaderBase::Initialize(device, hwnd, filePaths, layoutDesc, samplerDesc))
		return false;
	*/

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


bool ShaderVolumetric::SetShaderParameters(ID3D11DeviceContext* deviceContext, const Model& model, const Camera& camera, float elapsed)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WMBuffer* dataPtr;
	ViewRayBuffer* dataPtr2;
	VariableBuffer* dataPtr3;

	SMatrix mT = model.transform.Transpose();
	SMatrix vT = camera.GetViewMatrix().Transpose();
	SMatrix pT = camera.GetProjectionMatrix().Transpose();

	/*
	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr = (WMBuffer*)mappedResource.pData;	// Get a pointer to the data in the constant buffer.
	dataPtr->world = mT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);
	*/

	//view data - updates per frame
	if (FAILED(deviceContext->Map(_viewRayBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr2 = (ViewRayBuffer*)mappedResource.pData;
	dataPtr2->rot = SMatrix::CreateFromAxisAngle(SVec3(0, 1, 0), PI * 0.5f * elapsed);
	dataPtr2->ePos = Math::fromVec3(camera.GetCameraMatrix().Translation(), 1.0f);
	deviceContext->Unmap(_viewRayBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &_viewRayBuffer);

	/*
	deviceContext->IASetInputLayout(_layout);
	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &_sampleState);
	*/

	return true;
}
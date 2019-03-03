#include "ShaderVolumetric.h"
#include "Model.h"

ShaderVolumetric::ShaderVolumetric() : ShaderBase()
{
	timeElapsed = 0.f;
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

	return true;
}

bool ShaderVolumetric::setLightData(ID3D11DeviceContext* dc, const PointLight& pLight)
{
	LightBuffer* lightBufferPtr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	if (FAILED(dc->Map(_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	lightBufferPtr = (LightBuffer*)mappedResource.pData;
	lightBufferPtr->alc = pLight.alc;
	lightBufferPtr->ali = pLight.ali;
	lightBufferPtr->dlc = pLight.dlc;
	lightBufferPtr->dli = pLight.dli;
	lightBufferPtr->slc = pLight.slc;
	lightBufferPtr->sli = pLight.sli;
	lightBufferPtr->pos = pLight.pos;

	dc->Unmap(_lightBuffer, 0);
}



bool ShaderVolumetric::SetShaderParameters(ID3D11DeviceContext* deviceContext,
	Model& model, const SMatrix& v, const SMatrix& p, const SMatrix& cameraMatrix, float deltaTime)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBuffer* dataPtr;
	

	timeElapsed += deltaTime;

	SMatrix mT = model.transform.Transpose();
	SMatrix vT = v.Transpose();
	SMatrix pT = p.Transpose();

	SVec4 viewDir = -vT.Forward();	//@TODO CHECK IF THIS WORKS RIGHT!!! Inverse and transpose work similarly for view matrix... a bit of a hack tbh

	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;

	dataPtr = (MatrixBuffer*)mappedResource.pData;	// Get a pointer to the data in the constant buffer.
	dataPtr->world = mT;
	dataPtr->view = vT;
	dataPtr->projection = pT;

	deviceContext->Unmap(_matrixBuffer, 0);

	deviceContext->PSSetConstantBuffers(0, 1, &_lightBuffer);

	bufferNumber = 0;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &_matrixBuffer);

	deviceContext->IASetInputLayout(_layout);
	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &_sampleState);

	return true;
}
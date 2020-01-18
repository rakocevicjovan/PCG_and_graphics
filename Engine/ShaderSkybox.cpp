#include "ShaderSkybox.h"
#include "Model.h"
#include "Camera.h"

ShaderSkybox::ShaderSkybox()
{
	_vertexShader = 0;
	_pixelShader = 0;
	_layout = 0;
	_sampleState = 0;
	_matrixBuffer = 0;
}


ShaderSkybox::~ShaderSkybox()
{
	DECIMATE(_matrixBuffer);
	DECIMATE(_sampleState);
	DECIMATE(_layout);
	DECIMATE(_vertexShader);
	DECIMATE(_pixelShader);
}


bool ShaderSkybox::Initialize(const ShaderCompiler& shc,const std::vector<std::wstring> filePaths, 
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{
	bool result = true;

	this->_filePaths = filePaths;

	result &= shc.compileVS(filePaths.at(0), layoutDesc, _vertexShader, _layout);
	result &= shc.compilePS(filePaths.at(1), _pixelShader);
	result &= shc.createSamplerState(samplerDesc, _sampleState);

	D3D11_BUFFER_DESC matrixBufferDesc = shc.createCBufferDesc(sizeof(SMatrix));
	result &= shc.createConstantBuffer(matrixBufferDesc, _matrixBuffer);

	return true;
}



bool ShaderSkybox::SetShaderParameters(ID3D11DeviceContext* deviceContext, const Camera& c, float deltaTime, ID3D11ShaderResourceView* tex)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WMBuffer* dataPtr;

	if (FAILED(deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		return false;
	dataPtr = (WMBuffer*)mappedResource.pData;
	dataPtr->world = SMatrix::CreateTranslation(c.GetCameraMatrix().Translation()).Transpose();
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	deviceContext->IASetInputLayout(_layout);
	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &_sampleState);
	deviceContext->PSSetShaderResources(0, 1, &(tex));

	return true;
}



bool ShaderSkybox::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, &(_unbinder[0]));
	return true;
}
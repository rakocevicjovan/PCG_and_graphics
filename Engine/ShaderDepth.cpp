#include "ShaderDepth.h"
#include "Math.h"


ShaderDepth::ShaderDepth()
{
	_vertexShader = 0;
	_pixelShader = 0;
	_layout = 0;
	_matrixBuffer = 0;
}


ShaderDepth::~ShaderDepth()
{
}


bool ShaderDepth::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{
	bool result = true;

	_filePaths = filePaths;

	result &= shc.compileVS(filePaths.at(0), layoutDesc, _vertexShader, _layout);
	result &= shc.compilePS(filePaths.at(1), _pixelShader);
	result &= shc.createSamplerState(samplerDesc, _sampleState);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC mmBuffDesc = ShaderCompiler::createBufferDesc(sizeof(WMBuffer));
	result &= shc.createConstantBuffer(mmBuffDesc, _matrixBuffer);

	return result;
}



void ShaderDepth::ShutdownShader()
{
	DECIMATE(_vertexShader);
	DECIMATE(_pixelShader);
	DECIMATE(_matrixBuffer);
	DECIMATE(_layout);
}



bool ShaderDepth::SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& m)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WMBuffer* dataPtr;

	SMatrix mT = m.Transpose();

	// Lock the constant matrix buffer so it can be written to.
	result = deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	dataPtr = (WMBuffer*)mappedResource.pData;
	dataPtr->world = mT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);	// Now set the constant buffer in the vertex shader with the updated values.
	//END MATRIX BUFFER

	deviceContext->IASetInputLayout(_layout);
	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
		
	return true;
}
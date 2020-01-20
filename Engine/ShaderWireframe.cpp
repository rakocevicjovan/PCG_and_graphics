#include "ShaderWireframe.h"


WireframeShader::WireframeShader()
{
	_vertexShader = nullptr;
	_geometryShader = nullptr;
	_pixelShader = nullptr;
	_layout = nullptr;
	_matrixBuffer = nullptr;
}



WireframeShader::~WireframeShader()
{
	ShutdownShader();
}



bool WireframeShader::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{
	bool result = true;

	_filePaths = filePaths;

	result &= shc.compileVS(filePaths.at(0), layoutDesc, _vertexShader, _layout);
	result &= shc.compileGS(filePaths.at(1), _geometryShader);
	result &= shc.compilePS(filePaths.at(2), _pixelShader);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC mmBuffDesc = ShaderCompiler::createBufferDesc(sizeof(WMBuffer));
	result &= shc.createConstantBuffer(mmBuffDesc, _matrixBuffer);

	return result;
}



void WireframeShader::ShutdownShader()
{
	DECIMATE(_vertexShader)
	DECIMATE(_geometryShader)
	DECIMATE(_pixelShader)
	DECIMATE(_layout)
	DECIMATE(_matrixBuffer)
}



bool WireframeShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& mMat)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WMBuffer* dataPtr;

	SMatrix mT = mMat.Transpose();

	result = deviceContext->Map(_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;
	dataPtr = (WMBuffer*)mappedResource.pData;
	dataPtr->world = mT;
	deviceContext->Unmap(_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &_matrixBuffer);

	deviceContext->VSSetShader(_vertexShader, NULL, 0);
	deviceContext->GSSetShader(_geometryShader, NULL, 0);
	deviceContext->PSSetShader(_pixelShader, NULL, 0);
	deviceContext->IASetInputLayout(_layout);
	deviceContext->PSSetShaderResources(0, 1, &(_unbinder[0]));

	return true;
}



void WireframeShader::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	deviceContext->PSSetShaderResources(0, 1, NULL);
	deviceContext->GSSetShader(NULL, 0, 0);
}
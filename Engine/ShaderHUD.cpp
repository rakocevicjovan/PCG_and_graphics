#include "ShaderHUD.h"


ShaderHUD::ShaderHUD()
{
	_vertexShader = nullptr;
	_pixelShader = nullptr;
	_layout = nullptr;
}



ShaderHUD::~ShaderHUD()
{
	ShutdownShader();
}


// Doesn't have any constant buffers, uses clamp sample state
bool ShaderHUD::Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{
	bool result = true;

	_filePaths = filePaths;

	result &= shc.compileVS(filePaths.at(0), layoutDesc, _vertexShader, _layout);
	result &= shc.compilePS(filePaths.at(1), _pixelShader);
	result &= shc.createSamplerState(samplerDesc, _sampleState);

	return result;
}



void ShaderHUD::ShutdownShader()
{
	DECIMATE(_vertexShader)
	DECIMATE(_pixelShader)
	DECIMATE(_sampleState)
	DECIMATE(_layout)
}



bool ShaderHUD::SetShaderParameters(ID3D11DeviceContext* deviceContext)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	deviceContext->IASetInputLayout(_layout);
	deviceContext->VSSetShader(_vertexShader, 0, 0);
	deviceContext->PSSetShader(_pixelShader, 0, 0);
	deviceContext->PSSetSamplers(0, 1, &_sampleState);

	return true;
}



void ShaderHUD::ReleaseShaderParameters(ID3D11DeviceContext* deviceContext)
{
	//deviceContext->PSSetShaderResources(0, 1, &(_unbinder[0]));
}
#include "TerrainShader.h"

TerrainShader::TerrainShader() : ShaderBase()
{
}


TerrainShader::~TerrainShader()
{
}



bool TerrainShader::Initialize(ID3D11Device* device, HWND hwnd, const std::vector<std::wstring> filePaths,
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc)
{

	ShaderBase::Initialize(device, hwnd, filePaths, layoutDesc, samplerDesc);
	


	return true;
}



bool TerrainShader::SetShaderParameters(SPBase* spb)
{
	ShaderBase::SetShaderParameters(spb);

}

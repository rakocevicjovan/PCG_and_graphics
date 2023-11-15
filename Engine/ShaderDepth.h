#pragma once
#include "ShaderCompiler.h"
#include "ShaderDataStructs.h"

class ShaderDepth
{
public:
	ShaderDepth();
	~ShaderDepth();

	bool Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);
	bool SetShaderParameters(ID3D11DeviceContext*, const SMatrix&);
	void ShaderDepth::ShutdownShader();

	RenderFormat renderFormat;
	unsigned int texturesAdded = 0;

private:

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampleState;
	
	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	ID3D11Buffer* _matrixBuffer;

	std::vector<std::wstring> _filePaths;
};
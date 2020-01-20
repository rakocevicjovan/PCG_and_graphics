#pragma once
#include <vector>
#include <string>
#include "Math.h"
#include "Light.h"
#include "ShaderDataStructs.h"
#include "ShaderCompiler.h"


class ShaderHUD
{
public:
	ShaderHUD();
	~ShaderHUD();

	bool Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);
	bool SetShaderParameters(ID3D11DeviceContext*);
	void ReleaseShaderParameters(ID3D11DeviceContext*);
	void ShutdownShader();

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampleState;

	RenderFormat renderFormat;
	unsigned int texturesAdded = 0;

private:
	ID3D11ShaderResourceView* _unbinder[1] = { nullptr };
	std::vector<std::wstring> _filePaths;
};
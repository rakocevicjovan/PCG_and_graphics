#pragma once
#include <vector>
#include <string>
#include "Math.h"
#include "Light.h"
#include "ShaderDataStructs.h"
#include "ShaderCompiler.h"


class ShaderPT
{
public:
	ShaderPT();
	~ShaderPT();

	bool Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);
	bool SetShaderParameters(ID3D11DeviceContext*, const SMatrix& mMat, const SMatrix& vLight, const SMatrix& pLight,
		const DirectionalLight& dLight, const SVec3& eyePos, ID3D11ShaderResourceView* projectionTexture);
	void ReleaseShaderParameters(ID3D11DeviceContext*);
	void ShutdownShader();

private:

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampleState;

	ID3D11Buffer* _matrixBuffer;
	ID3D11Buffer* _vpBuffer;
	ID3D11Buffer* _lightBuffer;

	std::vector<std::wstring> _filePaths;

	ID3D11ShaderResourceView* _unbinder[1] = { nullptr };

	RenderFormat renderFormat;
};


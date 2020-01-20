#pragma once
#include <vector>
#include <string>
#include "Math.h"
#include "Light.h"
#include "CloudscapeDefinition.h"
#include "ShaderDataStructs.h"
#include "ShaderCompiler.h"

class Camera;

class ShaderStrife
{
	struct CloudBuffer
	{
		SVec4 lightPos;
		SVec4 lightRGBI;
		SVec4 extinction;
		SVec4 eyePosElapsed;
		SVec4 eccentricity;
		SVec4 repeat;
		SVec4 opt;
		SVec4 misc;
		SVec4 ALTop;
		SVec4 ALBot;

		SMatrix camMatrix;
	};

public:
	ShaderStrife();
	~ShaderStrife();

	bool Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, const Camera& cam, const Strife::CloudscapeDefinition& csDef, float elapsed);
	void ReleaseShaderParameters(ID3D11DeviceContext*);
	void ShutdownShader();

	RenderFormat renderFormat;
	UINT texturesAdded = 4;

private:

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	ID3D11Buffer* _matrixBuffer;
	ID3D11Buffer* _cloudBuffer;

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampleState;

	std::vector<std::wstring> _filePaths;

	ID3D11ShaderResourceView* _unbinder[1] = { nullptr };
};


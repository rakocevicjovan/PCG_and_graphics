#pragma once
#include <vector>
#include <string>
#include "Math.h"
#include "Light.h"
#include "ShaderCompiler.h"
#include "ShaderDataStructs.h"

class Model;
class Camera;

class ShaderSkybox
{
protected:

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;
	ID3D11Buffer* _matrixBuffer;
	std::vector<std::wstring> _filePaths;

	ID3D11ShaderResourceView* _unbinder[1] = { nullptr };


public:

	ShaderSkybox();
	~ShaderSkybox();

	bool Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths, std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);
	bool SetShaderParameters(ID3D11DeviceContext*, const Camera& c, float deltaTime, ID3D11ShaderResourceView* tex);
	bool ReleaseShaderParameters(ID3D11DeviceContext*);

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampleState;

	RenderFormat renderFormat;
	unsigned int texturesAdded = 1;
};
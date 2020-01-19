#pragma once
#include <vector>
#include <string>
#include "Math.h"
#include "Light.h"
#include "ShaderDataStructs.h"
#include "ShaderCompiler.h"

class Camera;

class ShaderCM
{
public:
	ShaderCM();
	~ShaderCM();

	bool Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);
	bool SetShaderParameters(ID3D11DeviceContext*, const SMatrix& mMat, const Camera& cam, const PointLight& dLight, float deltaTime, ID3D11ShaderResourceView* tex);
	bool ReleaseShaderParameters(ID3D11DeviceContext*);
	void ShutdownShader();

	RenderFormat renderFormat;
	unsigned int texturesAdded = 1;

private:
	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampleState;

	ID3D11Buffer* _matrixBuffer;
	ID3D11Buffer* _variableBuffer;
	ID3D11Buffer* _lightBuffer;

	std::vector<std::wstring> _filePaths;

	ID3D11ShaderResourceView* unbinder[1] = { nullptr };

};
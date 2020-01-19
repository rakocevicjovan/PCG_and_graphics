#pragma once
#include <vector>
#include <string>
#include "Math.h"
#include "Light.h"
#include "ShaderDataStructs.h"
#include "ShaderCompiler.h"

class Camera;


class ShaderClipper
{
	std::vector<std::wstring> _filePaths;

	struct ClipperBuffer
	{
		SVec4 clipper;
	};

public:
	ShaderClipper();
	~ShaderClipper();

	virtual bool Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);
	bool SetClipper(ID3D11DeviceContext* deviceContext, const SVec4& clipper);
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& mMat, const Camera& cam, const PointLight& pLight, float deltaTime);
	void ReleaseShaderParameters(ID3D11DeviceContext* deviceContext);

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampleState;

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	ID3D11Buffer* _matrixBuffer;
	ID3D11Buffer* _lightBuffer;
	ID3D11Buffer* _clipperBuffer;

	unsigned int texturesAdded = 0;
};
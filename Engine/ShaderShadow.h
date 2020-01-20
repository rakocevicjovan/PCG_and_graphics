#pragma once
#include <vector>
#include <string>
#include "Math.h"
#include "Light.h"
#include "ShaderDataStructs.h"
#include "ShaderCompiler.h"


//used to inherit from base @TODO
class ShaderShadow
{
protected:
	std::vector<std::wstring> _filePaths;
	ID3D11ShaderResourceView* _unbinder[1] = { nullptr };

public:
	ShaderShadow();
	~ShaderShadow();

	bool Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& mMat, const SMatrix& vLight,
		const SMatrix& pLight, const PointLight& dLight, const SVec3& eyePos, ID3D11ShaderResourceView* projectionTexture);

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampleStateWrap;
	ID3D11SamplerState* _sampleStateClamp;

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	//for vs
	ID3D11Buffer* _matrixBuffer;
	ID3D11Buffer* _shadowMatrixBuffer;

	//for ps
	ID3D11Buffer* _lightBuffer;
	
	RenderFormat renderFormat;
	unsigned int texturesAdded = 1;	//not robust
};
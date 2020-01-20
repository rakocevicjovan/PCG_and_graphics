#pragma once
#include <vector>
#include <string>
#include "Math.h"
#include "Light.h"
#include "MeshDataStructs.h"
#include "ShaderCompiler.h"


class ShaderSkelAnim
{
public:
	ShaderSkelAnim();
	~ShaderSkelAnim();

	bool Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& mMat, const SVec3& camPos, 
		const PointLight& pLight, const std::vector<SMatrix>& boneTransformsIn);
	void ReleaseShaderParameters(ID3D11DeviceContext*);
	void ShutdownShader();

	ID3D11SamplerState* _sampleState;

private:
	BoneTransformBuffer boneTransformBuffer;

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	ID3D11Buffer* _matrixBuffer;
	ID3D11Buffer* _bonesBuffer;
	ID3D11Buffer* _lightBuffer;

	ID3D11InputLayout* _layout;

	std::vector<std::wstring> _filePaths;

	ID3D11ShaderResourceView* _unbinder[1] = { nullptr };
};
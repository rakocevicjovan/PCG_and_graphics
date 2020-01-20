#pragma once
#include <vector>
#include <string>
#include "Math.h"
#include "Light.h"
#include "ShaderDataStructs.h"
#include "ShaderCompiler.h"


class ShaderWater
{
	struct WaterBuffer
	{
		SVec3 alc;
		float ali;

		SVec3  dlc;
		float dli;

		SVec3 slc;
		float sli;

		SVec4 dir;
		SVec4 eyePos;

		float elapsed;
		SVec3 padding;
	};

public:
	ShaderWater();
	~ShaderWater();

	bool Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& mMat, const SVec3& camPos, const PointLight& dirLight, float elapsed,
		ID3D11ShaderResourceView* whiteSRV, ID3D11ShaderResourceView* reflectionMap, ID3D11ShaderResourceView* refractionMap);
	bool ReleaseShaderParameters(ID3D11DeviceContext*);
	void ShutdownShader();
	
	RenderFormat renderFormat;
	UINT texturesAdded = 3;

private:
	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampleState;

	ID3D11Buffer* _matrixBuffer;
	ID3D11Buffer* _waterBuffer;

	std::vector<std::wstring> _filePaths;
	ID3D11ShaderResourceView* _unbinder[1] = { nullptr };
};


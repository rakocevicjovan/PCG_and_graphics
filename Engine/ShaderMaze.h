#pragma once
#include "Math.h"
#include "Light.h"
#include "ShaderCompiler.h"

class Texture;


//used to inherit from base @TODO
class ShaderMaze
{
public:
	ShaderMaze();
	~ShaderMaze();

	bool Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);
	bool SetShaderParameters(ID3D11DeviceContext*, const SMatrix& mMat, const SVec3& camPos, const PointLight& pLight, float deltaTime, const Texture& d, const Texture& n);
	void ReleaseShaderParameters(ID3D11DeviceContext* deviceContext);

protected:

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampleState;

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	ID3D11Buffer* _matrixBuffer;
	ID3D11Buffer* _mazePosBuffer;
	ID3D11Buffer* _lightBuffer;

	unsigned int texturesAdded = 2;

	std::vector<std::wstring> _filePaths;
	ID3D11ShaderResourceView* _unbinder[1] = { nullptr };
};
#pragma once
#include "ShaderDataStructs.h"
#include "ShaderCompiler.h"

class Camera;

//used to inherit from base @TODO
class ShaderVolumetric
{
public:
	ShaderVolumetric();
	~ShaderVolumetric();

	bool Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);
	
	bool SetShaderParameters(ID3D11DeviceContext*, const SMatrix& mMat, const Camera& camera, float elapsed);

	void ReleaseShaderParameters(ID3D11DeviceContext*);

private:

	std::vector<std::wstring> _filePaths;
	ID3D11ShaderResourceView* _unbinder[1] = { nullptr };

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampleState;

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	ID3D11Buffer* _matrixBuffer;
	ID3D11Buffer* _elapsedTimeBuffer;
	ID3D11Buffer* _volumShaderBuffer;

	RenderFormat renderFormat;		//belongs to vertex data, shouldn't be here
	unsigned int texturesAdded = 0;	//not robust
};
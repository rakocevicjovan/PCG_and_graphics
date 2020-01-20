#pragma once
#include "Camera.h"
#include "ShaderDataStructs.h"
#include "ShaderCompiler.h"


class VolumetricScreen
{
private:
	struct VolumetricScreenBuffer
	{
		SVec4 cameraPosition;	//and time elapsed for w
		SVec4 cameraDirection;
		SVec4 gale1;	//xz of tube, height, radius
		SVec4 gale2;
		SVec4 gale3;
		SVec4 gale4;
	};

	std::vector<std::wstring> _filePaths;
	ID3D11ShaderResourceView* _unbinder[1] = { nullptr };

public:
	VolumetricScreen();
	~VolumetricScreen();

	int texturesAdded = 0;

	Mesh* screenQuad;

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampleState;

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	ID3D11Buffer* _matrixBuffer;
	ID3D11Buffer* _volumScreenBuffer;

	bool Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, const Camera& camera, const SMatrix& gales, float elapsed);
	void ReleaseShaderParameters(ID3D11DeviceContext*);
};




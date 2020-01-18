#pragma once
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>
#include <vector>
#include <string>
#include "Math.h"
#include "Light.h"
#include "ShaderDataStructs.h"
#include "ShaderCompiler.h"

#define DECIMATE(x) if(x) { x->Release(); x = nullptr; }

class ShaderBase
{
protected:

	std::vector<std::wstring> _filePaths;
	ID3D11ShaderResourceView* _unbinder[1] = { nullptr };

public:
	ShaderBase();
	~ShaderBase();

	bool Initialize(const ShaderCompiler& shc, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);
	bool SetShaderParameters(ID3D11DeviceContext*, SMatrix& modelMat, const Camera& cam, const PointLight& pLight, float deltaTime);
	void ReleaseShaderParameters(ID3D11DeviceContext*);
	

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampleState;

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;

	ID3D11Buffer* _matrixBuffer;
	ID3D11Buffer* _lightBuffer;

	RenderFormat renderFormat;		//belongs to vertex data, shouldn't be here

	unsigned int texturesAdded = 0;	//not robust
};


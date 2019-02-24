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
#include "ShaderBase.h"
#include "ShaderDataStructs.h"

class Model;

class ShaderShadow : public ShaderBase 
{
protected:

	ID3D11Buffer* _lightBuffer2;

	std::vector<std::wstring> filePaths;

	ID3D11ShaderResourceView* unbinder[1] = { nullptr };

public:
	ShaderShadow();
	~ShaderShadow();

	bool Initialize(ID3D11Device*, HWND, const std::vector<std::wstring> filePaths);
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, Model& model, const SMatrix& v, const SMatrix& v2,
		const SMatrix& p, const SMatrix& p2, const PointLight& dLight, const SVec3& eyePos, ID3D11ShaderResourceView* projectionTexture);

	ID3D11SamplerState *_sampleStateClamp;
};


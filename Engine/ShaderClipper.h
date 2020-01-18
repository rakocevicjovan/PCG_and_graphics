#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>
#include <vector>
#include <string>
#include "Math.h"
#include "Light.h"
#include "ShaderDataStructs.h"
#include "ShaderBase.h"

#include "ShaderCompiler.h"

class Camera;


class ShaderClipper
{
	std::vector<std::wstring> filePaths;

	struct ClipperBuffer
	{
		SVec4 clipper;
	};

	ID3D11Buffer* _clipperBuffer;

public:
	ShaderClipper();
	~ShaderClipper();

	virtual bool Initialize(ID3D11Device*, HWND, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);
	bool SetClipper(ID3D11DeviceContext* deviceContext, const SVec4& clipper);
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, const SMatrix& mMat, const Camera& cam, const PointLight& pLight, float deltaTime);
	void ReleaseShaderParameters(ID3D11DeviceContext* deviceContext);

	unsigned int texturesAdded = 0;
};
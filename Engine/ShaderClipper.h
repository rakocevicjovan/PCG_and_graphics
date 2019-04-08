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
#include "ShaderBase.h"

class Model;
class Camera;


class ShaderClipper : public ShaderBase
{
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
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, Model& model, const Camera& cam, const PointLight& pLight, float deltaTime);
	void ReleaseShaderParameters(ID3D11DeviceContext* deviceContext);

	unsigned int texturesAdded = 0;
};


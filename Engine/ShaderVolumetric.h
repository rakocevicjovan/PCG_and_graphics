#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "ShaderBase.h"

class Model;

class ShaderVolumetric : public ShaderBase {

public:
	ShaderVolumetric();
	~ShaderVolumetric();

	bool Initialize(ID3D11Device*, HWND, const std::vector<std::wstring> filePaths, 
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);
	
	bool setLightData(ID3D11DeviceContext* dc, const PointLight& pLight);
	
	bool SetShaderParameters(ID3D11DeviceContext*, Model& m, const SMatrix& v, const SMatrix& p, const SMatrix& cameraMatrix, float deltaTime);

	

private:

	float timeElapsed;
};


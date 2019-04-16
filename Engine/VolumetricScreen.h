#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "ShaderBase.h"
#include "Camera.h"
#include "ShaderDataStructs.h"

class Mesh;


struct VolumetricScreenBuffer
{
	SVec4 cameraPosition;	//and time elapsed for w
	SVec4 cameraDirection;
	SVec4 gale1;	//xz of tube, height, radius
	SVec4 gale2;
	SVec4 gale3;
	SVec4 gale4;
};

class VolumetricScreen : public ShaderBase
{
private:
	ID3D11Buffer* _viewRayBuffer;

public:
	VolumetricScreen();
	~VolumetricScreen();

	int texturesAdded = 0;

	Mesh* screenQuad;

	bool Initialize(ID3D11Device* device, HWND hwnd, const std::vector<std::wstring> filePaths,
		std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc, const D3D11_SAMPLER_DESC& samplerDesc);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, const Camera& camera, const SMatrix& gales, float elapsed);
};




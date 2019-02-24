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

class ShaderLight : public ShaderBase
{

public:
	ShaderLight();
	~ShaderLight();

	bool SetShaderParameters(ID3D11DeviceContext*, Model& m, const SMatrix& v, const SMatrix& p, 
								const PointLight& dLight, const SVec3& eyePos, float deltaTime);
	bool ReleaseShaderParameters(ID3D11DeviceContext*);

private:
	std::vector<std::wstring> filePaths;
	ID3D11ShaderResourceView* unbinder[1] = { nullptr };
};
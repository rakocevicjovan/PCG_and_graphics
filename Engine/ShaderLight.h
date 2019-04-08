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

class ShaderLight : public ShaderBase
{

public:
	ShaderLight();
	~ShaderLight();

	bool SetShaderParameters(ID3D11DeviceContext*, Model& m, const Camera& cam, const PointLight& dLight, float deltaTime);
	void ReleaseShaderParameters(ID3D11DeviceContext*);
};
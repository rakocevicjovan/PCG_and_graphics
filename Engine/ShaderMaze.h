#pragma once
#include "Math.h"
#include "Light.h"
#include "ShaderDataStructs.h"
#include "ShaderBase.h"

class Camera;

class ShaderMaze : public ShaderBase
{
public:
	ShaderMaze();
	~ShaderMaze();

	unsigned int texturesAdded = 2;

	bool SetShaderParameters(ID3D11DeviceContext*, const Model& m, const Camera& cam, const PointLight& pLight, float deltaTime, const Texture& d, const Texture& n);

	void ReleaseShaderParameters(ID3D11DeviceContext* deviceContext);
};
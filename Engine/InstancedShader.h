#pragma once
#include "ShaderBase.h"

class InstancedShader : public ShaderBase
{

	InstancedShader();
	~InstancedShader();

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, Model& model, const SMatrix& v, const SMatrix& p,
		const PointLight& dLight, const SVec3& eyePos, float deltaTime);



};
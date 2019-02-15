#pragma once
#include <vector>

#include "ShaderDataStructs.h"
#include "ShaderBase.h"
#include "ShaderLight.h"
#include "ShaderDepth.h"
#include "ShaderShadow.h"
#include "ShaderHUD.h"
#include "ShaderWireframe.h"
#include "ShaderPT.h"
#include "ShaderCM.h"
#include "ShaderSkybox.h"
#include "ShaderStrife.h"
#include "ShaderWater.h"
#include "InstancedShader.h"

class ShaderManager
{
protected:
	ID3D11Device* _device;
	HWND hwnd;

	MatrixBuffer matrixBuffer;
	VariableBuffer variableBuffer;
	LightBuffer lightBuffer;

public:

	ShaderBase shaderBase;
	ShaderLight shaderLight;
	WireframeShader shaderWireframe;
	ShaderHUD shaderHUD;
	ShaderDepth shaderDepth;
	ShaderPT shaderPT;
	ShaderShadow shaderShadow;
	ShaderCM shaderCM;
	ShaderSkybox shaderSkybox;
	ShaderStrife shaderStrife;
	ShaderWater shaderWater;

	ShaderManager();
	~ShaderManager();

	ShaderParametersLight spl;

	void init(ID3D11Device* _device, HWND hwnd);
};


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
#include "ShaderVolumetric.h"
#include "InstancedShader.h"
#include "TerrainShader.h"
#include "PerlinShader.h"
#include "ShaderMaze.h"


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
	InstancedShader shaderInstanced;
	PerlinShader shaderPerlin;
	TerrainShader shaderTerMult;
	TerrainShader shaderTerNorm;
	TerrainShader shaderTree;
	ShaderMaze shaderMaze;

	//wisps
	ShaderVolumetric shVolumEarth;
	ShaderVolumetric shVolumFire;
	ShaderVolumetric shVolumWater;
	ShaderVolumetric shVolumAir;
	ShaderVolumetric shVolumLava;

	ShaderManager();
	~ShaderManager();

	void init(ID3D11Device* _device, HWND hwnd);
};

/*
ShaderParametersLight spl;

spl.deltaTime = dTime;
spl.deviceContext = _deviceContext;
spl.dLight = &(RES.pointLight);
spl.eyePos = &(_cam.GetCameraMatrix().Translation());
spl.model = &(RES.pSys._model);
spl.proj = &(_cam.GetProjectionMatrix());
spl.view = &(_cam.GetViewMatrix());
*/
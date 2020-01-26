#pragma once
#include <vector>

#include "SkelAnimShader.h"
#include "ShaderDataStructs.h"
#include "Phong.h"
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
#include "ShaderMaze.h"
#include "ShaderClipper.h"
#include "VolumetricScreen.h"



class ShaderManager
{
protected:
	ID3D11Device* _device;
	HWND hwnd;

	WMBuffer matrixBuffer;
	ElapsedTimeBuffer variableBuffer;
	LightBuffer lightBuffer;

public:

	ShaderManager();
	~ShaderManager();

	void init(ID3D11Device* _device, HWND hwnd);

	Phong light;
	ShaderSkybox skyboxShader;
	InstancedShader instanced;

	// These don't necessarily work right now need to be tested after the rework that happened to all shader code
	ShaderSkelAnim animator;
	WireframeShader wireframe;
	ShaderHUD HUD, brightMask, blurHor, blurVer, bloom;
	ShaderDepth depth;
	ShaderPT texProjector;
	ShaderShadow shadow;
	ShaderCM cubeMapShader;
	ShaderStrife strife;
	ShaderWater water;
	InstancedShader dragon;
	TerrainShader terrainMultiTex, terrainNormals, treeShader;
	ShaderMaze mazeMat, mazeFloorMat;
	ShaderClipper clipper;

	ShaderVolumetric shVolumEarth, shVolumFire, shVolumWater, shVolumAir, shVolumLava, shVolumTornado;
	VolumetricScreen shVolumScreen;
};
#pragma once
#include "ShaderCompiler.h"
#include "Shader.h"
#include <unordered_map>

class ShaderCache
{
	ShaderCompiler* _shc;
	std::unordered_map<std::string, VertexShader*> _vsMap;
	std::unordered_map<std::string, PixelShader*> _psMap;

public:
	void init(ShaderCompiler* shCompiler);
	
	VertexShader* getVertShader(const std::string& name);
	PixelShader* getPixShader(const std::string& name);

	bool addVertShader(const std::string& name, VertexShader* vs);
	bool addPixShader(const std::string& name, PixelShader* ps);

	//@TODO obviously, this is to be replaced, sooner the better
	void createAllShadersBecauseIAmTooLazyToMakeThisDataDriven();
};



/*
// Convert to new system as needed

// Old shader manager header
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

	// Old shader manager code

	ShaderCompiler shc;
	shc.init(&hwnd, device);

	///implemented, but not required for the game currently
	//cubeMapShader.Initialize(shc, { L"cubemapVS.hlsl",	L"cubemapPS.hlsl" }, ptn_layout, skbyoxSD);
	//depth.Initialize(shc,			{ L"depthVS.hlsl",		L"depthPS.hlsl" }, p_layout, regularSD);
	//shadow.Initialize(shc,		{ L"shadowvVS.hlsl",	L"shadowPS.hlsl" }, ptn_layout, regularSD, clampSD);
	//animator.Initialize(shc,		{ L"AnimaVS.hlsl", L"AnimaPS.hlsl" }, ptn_biw_layout, regularSD);
	//clipper.Initialize(shc,		{ L"clipperVS.hlsl",	L"clipperPS.hlsl" }, ptn_layout, regularSD);
	//water.Initialize(shc,			{ L"waterVS.hlsl",		L"waterPS.hlsl" }, ptnt_layout, waterSD);
	//texProjector.Initialize(shc,	{ L"projectTex.vs", L"projectTex.ps" }, ptn_layout, regularSD);
	//strife.Initialize(shc,		{ L"strifeVS.hlsl", L"strifePS.hlsl" }, ptn_layout, cloudSD);
	//mazeMat.Initialize(shc,		{ L"mazeVS.hlsl", L"mazePS.hlsl" }, ptnt_layout, regularSD);
	//mazeFloorMat.Initialize(shc,	{ L"mazefloorVS.hlsl", L"mazePS.hlsl" }, ptnt_layout, regularSD);
	//wireframe.Initialize(shc,		{ L"wireframeVS.hlsl", L"wireframeGS.hlsl", L"wireframePS.hlsl" }, ptn_layout, regularSD);

	//screen space shaders, post processing mostly
	//HUD.Initialize(shc,			{ L"screenspaceVS.hlsl", L"rektPS.hlsl" }, ptn_layout, clampSD);
	//brightMask.Initialize(shc,	{ L"screenspaceVS.hlsl", L"brightMaskPS.hlsl" }, ptn_layout, regularSD);
	//blurHor.Initialize(shc,		{ L"screenspaceVS.hlsl", L"blurHorPS.hlsl" }, ptn_layout, regularSD);
	//blurVer.Initialize(shc,		{ L"screenspaceVS.hlsl", L"blurVerPS.hlsl" }, ptn_layout, regularSD);
	//bloom.Initialize(shc,			{ L"screenspaceVS.hlsl", L"bloomPS.hlsl" }, ptn_layout, regularSD);

	//volumetric
	//shVolumEarth.Initialize(shc,		{ L"volumVS.hlsl", L"volumEarthPS.hlsl" }, ptn_layout, regularSD);
	//shVolumFire.Initialize(shc,		{ L"volumVS.hlsl", L"volumFirePS.hlsl" }, ptn_layout, regularSD);
	//shVolumWater.Initialize(shc,		{ L"volumVS.hlsl", L"volumWaterPS.hlsl" }, ptn_layout, regularSD);
	//shVolumAir.Initialize(shc,		{ L"volumVS.hlsl", L"volumAirPS.hlsl" }, ptn_layout, regularSD);
	//shVolumLava.Initialize(shc,		{ L"volumVS.hlsl", L"volumLavaPS.hlsl" }, ptn_layout, regularSD);
	//shVolumTornado.Initialize(shc,	{ L"volumVS.hlsl", L"volumTornadoPS.hlsl" }, ptn_layout, regularSD);
	//shVolumScreen.Initialize(_shc,	{ L"windpipeVS.hlsl", L"windpipePS.hlsl" }, ptn_layout, regularSD);

	//terrain shenanigans
	//terrainMultiTex.Initialize(shc, { L"mazefloorVS.hlsl", L"terrainPS.hlsl" }, ptnt_layout, regularSD);
	//terrainNormals.Initialize(shc, { L"mazefloorVS.hlsl", L"rerrainVNPS.hlsl" }, ptnt_layout, regularSD);

	//intanced shaders
	//dragon.Initialize(shc, { L"dragonVS.hlsl", L"dragonPS.hlsl" }, ptn_instanced_layout, regularSD, 100);
	*/
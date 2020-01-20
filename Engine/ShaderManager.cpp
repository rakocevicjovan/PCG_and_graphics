#include "ShaderManager.h"
#include "ShaderCompiler.h"


ShaderManager::ShaderManager()
{

}



ShaderManager::~ShaderManager()
{
}



void ShaderManager::init(ID3D11Device * device, HWND hwnd)
{
	_device = device;

	ShaderCompiler shc;
	shc.init(&hwnd, device);

	std::vector<D3D11_INPUT_ELEMENT_DESC> p_layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	std::vector<D3D11_INPUT_ELEMENT_DESC> ptn_layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	std::vector<D3D11_INPUT_ELEMENT_DESC> ptn_instanced_layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "WORLDMATRIX",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "WORLDMATRIX",	1, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "WORLDMATRIX",	2, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "WORLDMATRIX",	3, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};

	std::vector<D3D11_INPUT_ELEMENT_DESC> ptnt_layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	  0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};


	D3D11_SAMPLER_DESC regularSD = shc.createSamplerDesc();	//uses default settings, wrap all

	D3D11_SAMPLER_DESC skbyoxSD = shc.createSamplerDesc(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_NEVER);

	D3D11_SAMPLER_DESC clampSD = shc.createSamplerDesc(
		D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_ALWAYS, 0, D3D11_FLOAT32_MAX,
		D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP
	);

	D3D11_SAMPLER_DESC waterSD = shc.createSamplerDesc(
		D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_ALWAYS, 0, D3D11_FLOAT32_MAX,
		D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_CLAMP
	);

	//move all this to material cache in this way or another!

	light.Initialize(shc, { L"lightVS.hlsl", L"lightPS.hlsl" }, ptn_layout, regularSD);
	skyboxShader.Initialize(shc, { L"skyboxVS.hlsl", L"skyboxPS.hlsl" }, ptn_layout, skbyoxSD);
	
	//implemented, but not required for the game currently
	//cubeMapShader.Initialize(shc, { L"cubemapVS.hlsl",	L"cubemapPS.hlsl" }, ptn_layout, skbyoxSD);
	//depth.Initialize(shc,			{ L"depthVS.hlsl",		L"depthPS.hlsl" }, p_layout, regularSD);
	//clipper.Initialize(shc,		{ L"clipperVS.hlsl",	L"clipperPS.hlsl" }, ptn_layout, regularSD);
	//water.Initialize(shc,			{ L"waterVS.hlsl",		L"waterPS.hlsl" }, ptnt_layout, waterSD);
	//shadow.Initialize(shc,		{ L"shadowvVS.hlsl",	L"shadowPS.hlsl" }, ptn_layout, regularSD, clampSD);
	//HUD.Initialize(shc,			{ L"screenspaceVS.hlsl",L"rektPS.hlsl" }, ptn_layout, clampSD);
	//texProjector.Initialize(shc,	{ L"projectTex.vs", L"projectTex.ps" }, ptn_layout, regularSD);


	/*
	wireframe.Initialize(_device, hwnd, { L"wireframeVS.hlsl", L"wireframeGS.hlsl", L"wireframePS.hlsl" });
	animator.Initialize(_device, hwnd, { L"AnimaVS.hlsl", L"AnimaPS.hlsl" });
	strife.Initialize(_device, hwnd, { L"strifeVS.hlsl", L"strifePS.hlsl" });
	
	brightnessMasker.Initialize(_device, hwnd, { L"brightMaskVS.hlsl", L"brightMaskPS.hlsl" });
	blurHor.Initialize(_device, hwnd, { L"blurVS.hlsl", L"blurHorPS.hlsl" });
	blurVer.Initialize(_device, hwnd, { L"blurVS.hlsl", L"blurVerPS.hlsl" });
	bloom.Initialize(_device, hwnd, { L"bloomVS.hlsl", L"bloomPS.hlsl" });
	

	shVolumEarth.Initialize(_device, hwnd, { L"volumVS.hlsl", L"volumEarthPS.hlsl" }, sbLayout, sbSamplerDesc);
	shVolumFire.Initialize(_device, hwnd, { L"volumVS.hlsl", L"volumFirePS.hlsl" }, sbLayout, sbSamplerDesc);
	shVolumWater.Initialize(_device, hwnd, { L"volumVS.hlsl", L"volumWaterPS.hlsl" }, sbLayout, sbSamplerDesc);
	shVolumAir.Initialize(_device, hwnd, { L"volumVS.hlsl", L"volumAirPS.hlsl" }, sbLayout, sbSamplerDesc);

	//other volumetric
	shVolumLava.Initialize(_device, hwnd, { L"volumVS.hlsl", L"volumLavaPS.hlsl" }, sbLayout, sbSamplerDesc);
	shVolumTornado.Initialize(_device, hwnd, { L"volumVS.hlsl", L"volumTornadoPS.hlsl" }, sbLayout, sbSamplerDesc);
	shVolumScreen.Initialize(_device, hwnd, { L"windpipeVS.hlsl", L"windpipePS.hlsl" }, sbLayout, sbSamplerDesc);	//it's a screen quad volumetric


	dynamicHeightMaze.Initialize(_device, hwnd, { L"mazeVS.hlsl", L"mazePS.hlsl" }, extendedLayout, sbSamplerDesc);
	normalMapper.Initialize(_device, hwnd, { L"mazefloorVS.hlsl", L"mazePS.hlsl" }, extendedLayout, sbSamplerDesc);
	treeShader.Initialize(_device, hwnd, { L"mazefloorVS.hlsl", L"terrainPS.hlsl" }, extendedLayout, sbSamplerDesc);
	terrainMultiTex.Initialize(_device, hwnd, { L"mazefloorVS.hlsl", L"terrainPS.hlsl" }, extendedLayout, sbSamplerDesc);
	terrainNormals.Initialize(_device, hwnd, { L"mazefloorVS.hlsl", L"normalsTerrainPS.hlsl" }, extendedLayout, sbSamplerDesc);


	perlin.Initialize(_device, hwnd, { L"perlin3dVS.hlsl", L"perlin3dPS.hlsl" }, sbLayout, sbSamplerDesc);
	instanced.Initialize(_device, hwnd, { L"InstancedVS.hlsl", L"InstancedPS.hlsl" }, instancedLayout, sbSamplerDesc, 5000);
	//dragon.Initialize(_device, hwnd, { L"dragonVS.hlsl", L"dragonPS.hlsl" }, instancedLayout, sbSamplerDesc, 100);
	*/
}
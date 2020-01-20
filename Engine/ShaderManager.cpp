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

	std::vector<D3D11_INPUT_ELEMENT_DESC> ptn_biw_layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_ID" , 0, DXGI_FORMAT_R32G32B32A32_UINT,		0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_W"  , 0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};


	D3D11_SAMPLER_DESC regularSD = shc.createSamplerDesc();	//uses default settings, wrap all

	D3D11_SAMPLER_DESC skbyoxSD = shc.createSamplerDesc(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_NEVER);

	D3D11_SAMPLER_DESC clampSD = shc.createSamplerDesc(
		D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_ALWAYS, 0, D3D11_FLOAT32_MAX,
		D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP);

	D3D11_SAMPLER_DESC waterSD = shc.createSamplerDesc(
		D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_ALWAYS, 0, D3D11_FLOAT32_MAX,
		D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_CLAMP);

	D3D11_SAMPLER_DESC cloudSD = shc.createSamplerDesc(
		D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_ALWAYS, 0.f, 8.f);



	//move all this to material cache in this way or another!

	light.Initialize(shc,			{ L"lightVS.hlsl", L"lightPS.hlsl" }, ptn_layout, regularSD);
	skyboxShader.Initialize(shc,	{ L"skyboxVS.hlsl", L"skyboxPS.hlsl" }, ptn_layout, skbyoxSD);
	

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
	//HUD.Initialize(shc,			{ L"screenspaceVS.hlsl",L"rektPS.hlsl" }, ptn_layout, clampSD);
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
	//instanced.Initialize(shc, { L"InstancedVS.hlsl", L"InstancedPS.hlsl" }, ptn_instanced_layout, regularSD, 5000);
	//dragon.Initialize(shc, { L"dragonVS.hlsl", L"dragonPS.hlsl" }, ptn_instanced_layout, regularSD, 100);
}
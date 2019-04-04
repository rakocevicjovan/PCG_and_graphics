#include "ShaderManager.h"



ShaderManager::ShaderManager()
{

}



ShaderManager::~ShaderManager()
{
}



void ShaderManager::init(ID3D11Device * device, HWND hwnd)
{
	_device = device;

	D3D11_SAMPLER_DESC sbSamplerDesc;
	std::vector<D3D11_INPUT_ELEMENT_DESC> sbLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	//with wrap
	ZeroMemory(&sbSamplerDesc, sizeof(sbSamplerDesc));
	sbSamplerDesc = { D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP,
		0.0f, 1, D3D11_COMPARISON_ALWAYS, 0, 0, 0, 0, 0, D3D11_FLOAT32_MAX };


	std::vector<std::wstring> shaderBaseNames = { L"lightvs.hlsl", L"lightps.hlsl" };
	shaderBase.Initialize(_device, hwnd, shaderBaseNames, sbLayout, sbSamplerDesc);

	std::vector<std::wstring> names = { L"lightvs.hlsl", L"lightps.hlsl" };
	shaderLight.Initialize(_device, hwnd, names, sbLayout, sbSamplerDesc);

	std::vector<std::wstring> wfsNames = { L"wireframe.vs", L"wireframe.gs", L"wireframe.ps" };
	shaderWireframe.Initialize(_device, hwnd, wfsNames);

	std::vector<std::wstring> hudNames = { L"rekt.vs", L"rekt.ps" };
	shaderHUD.Initialize(_device, hwnd, hudNames);

	std::vector<std::wstring> depthNames = { L"depth.vs", L"depth.ps" };
	shaderDepth.Initialize(_device, hwnd, depthNames);

	std::vector<std::wstring> projTexNames = { L"projectTex.vs", L"projectTex.ps" };
	shaderPT.Initialize(_device, hwnd, projTexNames);

	std::vector<std::wstring> shadowNames = { L"shadowvs.hlsl", L"shadowps.hlsl" };
	shaderShadow.Initialize(_device, hwnd, shadowNames);

	std::vector<std::wstring> cubeMapNames = { L"cubemap.vs", L"cubemap.ps" };
	shaderCM.Initialize(_device, hwnd, cubeMapNames);

	std::vector<std::wstring> skyboxNames = { L"skyboxvs.hlsl", L"skyboxps.hlsl" };
	shaderSkybox.Initialize(_device, hwnd, skyboxNames);

	std::vector<std::wstring> strifeNames = { L"strifevs.hlsl", L"strifeps.hlsl" };
	shaderStrife.Initialize(_device, hwnd, strifeNames);

	std::vector<std::wstring> waterNames = { L"Watervs.hlsl", L"Waterps.hlsl" };
	shaderWater.Initialize(_device, hwnd, waterNames);

	//4 sprites, uncomment upon implementing
	std::vector<std::wstring> vEarthNames = { L"volumVS.hlsl", L"volumEarthPS.hlsl" };
	shVolumEarth.Initialize(_device, hwnd, vEarthNames, sbLayout, sbSamplerDesc);
	
	std::vector<std::wstring> vFireNames = { L"volumVS.hlsl", L"volumFirePS.hlsl" };
	shVolumFire.Initialize(_device, hwnd, vFireNames, sbLayout, sbSamplerDesc);

	std::vector<std::wstring> vWaterNames = { L"volumVS.hlsl", L"volumWaterPS.hlsl" };
	shVolumWater.Initialize(_device, hwnd, vWaterNames, sbLayout, sbSamplerDesc);

	std::vector<std::wstring> vAirNames = { L"volumVS.hlsl", L"volumAirPS.hlsl" };
	shVolumAir.Initialize(_device, hwnd, vAirNames, sbLayout, sbSamplerDesc);

	//maze, trees and terrain neeed the extended layout for normal mapping
	std::vector<D3D11_INPUT_ELEMENT_DESC> extendedLayout = sbLayout;
	extendedLayout.push_back({ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 });

	std::vector<std::wstring> mazeNames = { L"mazeVS.hlsl", L"mazePS.hlsl" };
	shaderMaze.Initialize(_device, hwnd, mazeNames, extendedLayout, sbSamplerDesc);

	std::vector<std::wstring> volTreeNames = { L"mazefloorVS.hlsl", L"terrainPS.hlsl" };
	shaderTree.Initialize(_device, hwnd, volTreeNames, extendedLayout, sbSamplerDesc);

	std::vector<std::wstring> terrainNames = { L"mazefloorVS.hlsl", L"terrainPS.hlsl" };
	shaderTerMult.Initialize(_device, hwnd, terrainNames, extendedLayout, sbSamplerDesc);

	std::vector<std::wstring> terrainNames2 = { L"mazefloorVS.hlsl", L"normalsTerrainPS.hlsl" };
	shaderTerNorm.Initialize(_device, hwnd, terrainNames2, extendedLayout, sbSamplerDesc);

	//with clamp
	sbSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sbSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sbSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	std::vector<std::wstring> perlinNames = { L"perlin3dVS.hlsl", L"perlin3dPS.hlsl" };
	shaderPerlin.Initialize(_device, hwnd, perlinNames, sbLayout, sbSamplerDesc);

	
	//with instancing
	std::vector<D3D11_INPUT_ELEMENT_DESC> instancedLayout =
	{
		// Data from the vertex buffer
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "WORLDMATRIX",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "WORLDMATRIX",	1, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "WORLDMATRIX",	2, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "WORLDMATRIX",	3, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};

	std::vector<std::wstring> instancedNames = { L"InstancedVS.hlsl", L"InstancedPS.hlsl" };
	shaderInstanced.Initialize(_device, hwnd, instancedNames, instancedLayout, sbSamplerDesc, 100);
}
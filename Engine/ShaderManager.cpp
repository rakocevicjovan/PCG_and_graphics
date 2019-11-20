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
	ZeroMemory(&sbSamplerDesc, sizeof(sbSamplerDesc));
	sbSamplerDesc = { D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP,
		0.0f, 1, D3D11_COMPARISON_ALWAYS, 0, 0, 0, 0, 0, D3D11_FLOAT32_MAX };

	std::vector<D3D11_INPUT_ELEMENT_DESC> sbLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0}
		//,{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	
	/*base.Initialize(_device, hwnd, { L"lightvs.hlsl", L"lightps.hlsl" }, sbLayout, sbSamplerDesc);
	clipper.Initialize(_device, hwnd, { L"clipperVS.hlsl", L"clipperPS.hlsl" }, sbLayout, sbSamplerDesc);
	water.Initialize(_device, hwnd, { L"waterVS.hlsl", L"waterPS.hlsl" });
	depth.Initialize(_device, hwnd, { L"depth.vs", L"depth.ps" });
	texProjector.Initialize(_device, hwnd, { L"projectTex.vs", L"projectTex.ps" });
	shadow.Initialize(_device, hwnd, { L"shadowvs.hlsl", L"shadowps.hlsl" });
	*/

	wireframe.Initialize(_device, hwnd, { L"wireframe.vs", L"wireframe.gs", L"wireframe.ps" });

	animator.Initialize(_device, hwnd, { L"AnimaVS.hlsl", L"AnimaPS.hlsl" });
	light.Initialize(_device, hwnd, { L"lightvs.hlsl", L"lightps.hlsl" }, sbLayout, sbSamplerDesc);
	//cubeMapShader.Initialize(_device, hwnd, { L"cubemap.vs", L"cubemap.ps" });
	skyboxShader.Initialize(_device, hwnd, { L"skyboxvs.hlsl", L"skyboxps.hlsl" });
	strife.Initialize(_device, hwnd, { L"strifevs.hlsl", L"strifeps.hlsl" });

	//ui and post processing
	HUD.Initialize(_device, hwnd, { L"rekt.vs", L"rekt.ps" });
	
	/*brightnessMasker.Initialize(_device, hwnd, { L"brightMaskVS.hlsl", L"brightMaskPS.hlsl" });
	blurHor.Initialize(_device, hwnd, { L"blurVS.hlsl", L"blurHorPS.hlsl" });
	blurVer.Initialize(_device, hwnd, { L"blurVS.hlsl", L"blurVerPS.hlsl" });
	bloom.Initialize(_device, hwnd, { L"bloomVS.hlsl", L"bloomPS.hlsl" });
	

	//4 sprites, uncomment upon implementing
	shVolumEarth.Initialize(_device, hwnd, { L"volumVS.hlsl", L"volumEarthPS.hlsl" }, sbLayout, sbSamplerDesc);
	shVolumFire.Initialize(_device, hwnd, { L"volumVS.hlsl", L"volumFirePS.hlsl" }, sbLayout, sbSamplerDesc);
	shVolumWater.Initialize(_device, hwnd, { L"volumVS.hlsl", L"volumWaterPS.hlsl" }, sbLayout, sbSamplerDesc);
	shVolumAir.Initialize(_device, hwnd, { L"volumVS.hlsl", L"volumAirPS.hlsl" }, sbLayout, sbSamplerDesc);

	//other volumetric
	shVolumLava.Initialize(_device, hwnd, { L"volumVS.hlsl", L"volumLavaPS.hlsl" }, sbLayout, sbSamplerDesc);
	shVolumTornado.Initialize(_device, hwnd, { L"volumVS.hlsl", L"volumTornadoPS.hlsl" }, sbLayout, sbSamplerDesc);
	shVolumScreen.Initialize(_device, hwnd, { L"windpipeVS.hlsl", L"windpipePS.hlsl" }, sbLayout, sbSamplerDesc);	//it's a screen quad volumetric


	//maze, trees and terrain neeed the extended layout for normal mapping
	std::vector<D3D11_INPUT_ELEMENT_DESC> extendedLayout = sbLayout;
	extendedLayout.push_back({ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 });

	dynamicHeightMaze.Initialize(_device, hwnd, { L"mazeVS.hlsl", L"mazePS.hlsl" }, extendedLayout, sbSamplerDesc);
	normalMapper.Initialize(_device, hwnd, { L"mazefloorVS.hlsl", L"mazePS.hlsl" }, extendedLayout, sbSamplerDesc);
	treeShader.Initialize(_device, hwnd, { L"mazefloorVS.hlsl", L"terrainPS.hlsl" }, extendedLayout, sbSamplerDesc);
	terrainMultiTex.Initialize(_device, hwnd, { L"mazefloorVS.hlsl", L"terrainPS.hlsl" }, extendedLayout, sbSamplerDesc);
	terrainNormals.Initialize(_device, hwnd, { L"mazefloorVS.hlsl", L"normalsTerrainPS.hlsl" }, extendedLayout, sbSamplerDesc);

	//with clamp
	sbSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sbSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sbSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	perlin.Initialize(_device, hwnd, { L"perlin3dVS.hlsl", L"perlin3dPS.hlsl" }, sbLayout, sbSamplerDesc);

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

	instanced.Initialize(_device, hwnd, { L"InstancedVS.hlsl", L"InstancedPS.hlsl" }, instancedLayout, sbSamplerDesc, 100);
	dragon.Initialize(_device, hwnd, { L"dragonVS.hlsl", L"dragonPS.hlsl" }, instancedLayout, sbSamplerDesc, 100);
	*/
}
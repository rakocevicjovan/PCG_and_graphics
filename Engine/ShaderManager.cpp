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

	D3D11_INPUT_ELEMENT_DESC sbLayout[] =
	{
		// Data from the vertex buffer
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},

		// Data from the instance buffer
		{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT,    1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "INSTANCECOLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};

	D3D11_SAMPLER_DESC sbSamplerDesc;
	ZeroMemory(&sbSamplerDesc, sizeof(sbSamplerDesc));
	sbSamplerDesc = { D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP,
		0.0f, 1, D3D11_COMPARISON_ALWAYS, 0, 0, 0, 0, 0, D3D11_FLOAT32_MAX };


	std::vector<std::wstring> shaderBaseNames;
	shaderBaseNames.push_back(L"lightvs.hlsl");
	shaderBaseNames.push_back(L"lightps.hlsl");
	shaderBase.Initialize(_device, hwnd, shaderBaseNames, sbLayout, 5u, sbSamplerDesc);

	std::vector<std::wstring> names;
	names.push_back(L"lightvs.hlsl");
	names.push_back(L"lightps.hlsl");
	shaderLight.Initialize(_device, hwnd, names, sbLayout, 3u, sbSamplerDesc);

	std::vector<std::wstring> wfsNames;
	wfsNames.push_back(L"wireframe.vs");
	wfsNames.push_back(L"wireframe.gs");
	wfsNames.push_back(L"wireframe.ps");
	shaderWireframe.Initialize(_device, hwnd, wfsNames);

	std::vector<std::wstring> hudNames;
	hudNames.push_back(L"rekt.vs");
	hudNames.push_back(L"rekt.ps");
	shaderHUD.Initialize(_device, hwnd, hudNames);

	std::vector<std::wstring> depthNames;
	depthNames.push_back(L"depth.vs");
	depthNames.push_back(L"depth.ps");
	shaderDepth.Initialize(_device, hwnd, depthNames);

	std::vector<std::wstring> projTexNames;
	projTexNames.push_back(L"projectTex.vs");
	projTexNames.push_back(L"projectTex.ps");
	shaderPT.Initialize(_device, hwnd, projTexNames);

	std::vector<std::wstring> shadowNames;
	shadowNames.push_back(L"shadowvs.hlsl");
	shadowNames.push_back(L"shadowps.hlsl");
	shaderShadow.Initialize(_device, hwnd, shadowNames);

	std::vector<std::wstring> cubeMapNames;
	cubeMapNames.push_back(L"cubemap.vs");
	cubeMapNames.push_back(L"cubemap.ps");
	shaderCM.Initialize(_device, hwnd, cubeMapNames);

	std::vector<std::wstring> skyboxNames;
	skyboxNames.push_back(L"skyboxvs.hlsl");
	skyboxNames.push_back(L"skyboxps.hlsl");
	shaderSkybox.Initialize(_device, hwnd, skyboxNames);

	std::vector<std::wstring> strifeNames;
	strifeNames.push_back(L"strifevs.hlsl");
	strifeNames.push_back(L"strifeps.hlsl");
	shaderStrife.Initialize(_device, hwnd, strifeNames);

	std::vector<std::wstring> waterNames;
	waterNames.push_back(L"Watervs.hlsl");
	waterNames.push_back(L"Waterps.hlsl");
	shaderWater.Initialize(_device, hwnd, waterNames);
}
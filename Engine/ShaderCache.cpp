#include "pch.h"
#include "ShaderCache.h"
#include "ShaderDataStructs.h"
#include "Math.h"
#include "Sampler.h"
#include "CBuffer.h"
#include "FileUtilities.h"



#define NUM_CASCADES 3u	// Absolutely get this out of here



void ShaderCache::init(ShaderCompiler* shCompiler)
{
	_shc = shCompiler;
}



void ShaderCache::createAllShadersBecauseIAmTooLazyToMakeThisDataDriven()
{
	/// Shader initialization data

	// Vertex input layouts
	std::vector<D3D11_INPUT_ELEMENT_DESC> p_layout =
	{
		{ "POSITION", 0, static_cast<DXGI_FORMAT>(6), 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }	//DXGI_FORMAT_R32G32B32_FLOAT
	};

	std::vector<D3D11_INPUT_ELEMENT_DESC> pt_layout =
	{
		{ "POSITION", 0, static_cast<DXGI_FORMAT>(6), 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	std::vector<D3D11_INPUT_ELEMENT_DESC> ptn_layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	std::vector<D3D11_INPUT_ELEMENT_DESC> ptnt_layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	  0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

	std::vector<D3D11_INPUT_ELEMENT_DESC> ptn_biw_layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_ID" , 0, DXGI_FORMAT_R32G32B32A32_UINT,		0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_W"  , 0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};


	// Sampler descriptions

	D3D11_SAMPLER_DESC regularSD = Sampler::createSamplerDesc();	//uses default settings, wrap all

	D3D11_SAMPLER_DESC skbyoxSD = Sampler::createSamplerDesc(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_NEVER);

	D3D11_SAMPLER_DESC clampSD = Sampler::createSamplerDesc(
		D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_ALWAYS, 0, D3D11_FLOAT32_MAX,
		D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP);

	D3D11_SAMPLER_DESC waterSD = Sampler::createSamplerDesc(
		D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_ALWAYS, 0, D3D11_FLOAT32_MAX,
		D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_CLAMP);

	D3D11_SAMPLER_DESC cloudSD = Sampler::createSamplerDesc(
		D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_ALWAYS, 0.f, 8.f);


	// Constant buffer descriptions, although it can be reflected

	D3D11_BUFFER_DESC WMBufferDesc = CBuffer::createDesc(sizeof(WMBuffer));
	CBufferMeta WMBufferMeta(0, WMBufferDesc.ByteWidth);
	WMBufferMeta.addFieldDescription(CBUFFER_FIELD_CONTENT::TRANSFORM, 0, sizeof(WMBuffer));

	D3D11_BUFFER_DESC lightBufferDesc = CBuffer::createDesc(sizeof(LightBuffer));
	CBufferMeta lightBufferMeta(0, lightBufferDesc.ByteWidth);
	//lightBufferMeta.addFieldDescription(CBUFFER_FIELD_CONTENT::P_LIGHT, 0, sizeof(LightBuffer));

	/*
	D3D11_BUFFER_DESC shadowBufferDesc = ShaderCompiler::createBufferDesc(sizeof(CSMBuffer<NUM_CASCADES>));
	CBufferMeta shadowBufferMeta(11, shadowBufferDesc.ByteWidth);
	shadowBufferMeta.addFieldDescription(CBUFFER_FIELD_CONTENT::CSM, 0, sizeof(CSMBuffer<NUM_CASCADES>));
	*/

	/// VERTEX SHADERS

	// PTN vertex shader
	VertexShader* basicVS = new VertexShader(*_shc, L"lightVS.hlsl", ptn_layout, { WMBufferDesc });
	basicVS->describeBuffers({ WMBufferMeta });
	addVertShader("basicVS", basicVS);

	// Skybox vertex shader, uses xyww trick for infinite distance
	VertexShader* skyboxVS = new VertexShader(*_shc, L"skyboxVS.hlsl", ptn_layout, { WMBufferDesc });
	skyboxVS->describeBuffers({ WMBufferMeta });
	addVertShader("skyboxVS", skyboxVS);

	// Instanced vertex shader- @TODO determine where to put instancing VB and how to handle updating it...
	VertexShader* instancedVS = new VertexShader(*_shc, L"InstancedVS.hlsl", ptn_instanced_layout, { WMBufferDesc });
	instancedVS->describeBuffers({ WMBufferMeta });
	addVertShader("instancedVS", instancedVS);

	VertexShader* csmVS = new VertexShader(*_shc, L"CSMVS.hlsl", p_layout, { WMBufferDesc });
	csmVS->describeBuffers({ WMBufferMeta });
	addVertShader("csmVS", csmVS);

	VertexShader* csmSceneVS = new VertexShader(*_shc, L"csmSceneVS.hlsl", ptn_layout, { WMBufferDesc });
	csmSceneVS->describeBuffers({ WMBufferMeta });
	addVertShader("csmSceneVS", csmSceneVS);

	//VertexShader* hudVS = new VertexShader(*_shc, L"screenspaceVS.hlsl", pt_layout, {});	addVertShader("hudVS", hudVS);


	/// PIXEL SHADERS

	// Phong
	PixelShader* phong = new PixelShader(*_shc, L"lightPS.hlsl", { regularSD }, {});
	//phong->describeBuffers({ lightBufferMeta });
	addPixShader("phongPS", phong);

	// Skybox ps, special sampler, no lights
	PixelShader* skyboxPS = new PixelShader(*_shc, L"skyboxPS.hlsl", { skbyoxSD }, {});
	addPixShader("skyboxPS", skyboxPS);

	// PBR shader
	PixelShader* CookTorrance = new PixelShader(*_shc, L"CookTorrancePS.hlsl", { regularSD }, { lightBufferDesc });
	CookTorrance->describeBuffers({lightBufferMeta});
	addPixShader("CookTorrancePS", CookTorrance);

	//PixelShader* hudPS = new PixelShader(*_shc, L"hudPS.hlsl", clampSD, {});	addPixShader("hudPS", hudPS);

	// CSM Scene shader
	PixelShader* csmScenePs = new PixelShader(*_shc, L"csmScenePS.hlsl", { regularSD }, { });
	//csmScenePs->describeBuffers({ lightBufferMeta });
	addPixShader("csmScenePS", csmScenePs);

	PixelShader* clusterDebugPs = new PixelShader(*_shc, L"clusterDebug.hlsl", { regularSD }, {});
	addPixShader("clusterDebugPS", clusterDebugPs);

	PixelShader* clusterPs = new PixelShader(*_shc, L"ClusteredPS.hlsl", { regularSD }, {});
	addPixShader("clusterPS", clusterPs);
}



VertexShader* ShaderCache::getVertShader(const std::string& name)
{
	std::unordered_map<std::string, VertexShader*>::const_iterator found = _vsMap.find(name);
	if (found == _vsMap.end())
		assert(false && "VERTEX SHADER NOT FOUND!");
	else
		return found->second;
}



PixelShader* ShaderCache::getPixShader(const std::string& name)
{
	std::unordered_map<std::string, PixelShader*>::const_iterator found = _psMap.find(name);
	if (found == _psMap.end())
		assert(false && "PIXEL SHADER NOT FOUND!");
	else
		return found->second;
}



bool ShaderCache::addVertShader(const std::string& name, VertexShader* vs)
{
	return _vsMap.insert(std::unordered_map<std::string, VertexShader*>::value_type(name, vs)).second;
}



bool ShaderCache::addPixShader(const std::string& name, PixelShader* ps)
{
	return _psMap.insert(std::unordered_map<std::string, PixelShader*>::value_type(name, ps)).second;
}
#include "pch.h"
#include "ShaderCache.h"
#include "ShaderDataStructs.h"
#include "Sampler.h"
#include "CBuffer.h"
#include "FileUtilities.h"


void ShaderCache::init() {}


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


void ShaderCache::createAllShadersBecauseIAmTooLazyToMakeThisDataDriven(ShaderCompiler* shc)
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

	D3D11_SAMPLER_DESC regularSD = Sampler::CreateSamplerDesc();	//uses default settings, wrap all

	D3D11_SAMPLER_DESC skbyoxSD = Sampler::CreateSamplerDesc(
		D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 0., FLT_MAX, 0., 1, D3D11_COMPARISON_ALWAYS);

	D3D11_SAMPLER_DESC clampSD = Sampler::CreateSamplerDesc(
		D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP);

	D3D11_SAMPLER_DESC waterSD = Sampler::CreateSamplerDesc(
		D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_CLAMP);

	D3D11_SAMPLER_DESC cloudSD = Sampler::CreateSamplerDesc(
		D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 0.f, 8.f);


	// Constant buffer descriptions, although it can be reflected

	D3D11_BUFFER_DESC WMBufferDesc = CBuffer::createDesc(sizeof(WMBuffer));
	CBufferMeta WMBufferMeta(0, WMBufferDesc.ByteWidth);
	WMBufferMeta.addFieldDescription(0, sizeof(SMatrix));

	D3D11_BUFFER_DESC lightBufferDesc = CBuffer::createDesc(sizeof(LightBuffer));
	CBufferMeta lightBufferMeta(0, lightBufferDesc.ByteWidth);


#define SH_BASE_PATH L"Shaders/"
	/// VERTEX SHADERS

	// PTN vertex shader
	VertexShader* basicVS = new VertexShader(*shc, SH_BASE_PATH"lightVS.hlsl", ptn_layout, { WMBufferDesc });
	basicVS->describeBuffers({ WMBufferMeta });
	addVertShader("basicVS", basicVS);

	// Instanced vertex shader- @TODO determine where to put instancing VB and how to handle updating it...
	VertexShader* instancedVS = new VertexShader(*shc, SH_BASE_PATH"InstancedVS.hlsl", ptn_instanced_layout, { WMBufferDesc });
	instancedVS->describeBuffers({ WMBufferMeta });
	addVertShader("instancedVS", instancedVS);

	VertexShader* csmVS = new VertexShader(*shc, SH_BASE_PATH"CSMVS.hlsl", p_layout, { WMBufferDesc });
	csmVS->describeBuffers({ WMBufferMeta });
	addVertShader("csmVS", csmVS);

	VertexShader* csmSceneVS = new VertexShader(*shc, SH_BASE_PATH"csmSceneVS.hlsl", ptn_layout, { WMBufferDesc });
	csmSceneVS->describeBuffers({ WMBufferMeta });
	addVertShader("csmSceneVS", csmSceneVS);

	VertexShader* FSTriangleVS = new VertexShader(*shc, SH_BASE_PATH"FullScreenTriNoBufferVS.hlsl", {}, {});
	addVertShader("FSTriangleVS", FSTriangleVS);


	/// PIXEL SHADERS

	// Phong
	PixelShader* phong = new PixelShader(*shc, SH_BASE_PATH"lightPS.hlsl", { regularSD }, {});
	//phong->describeBuffers({ lightBufferMeta });
	addPixShader("phongPS", phong);

	// PBR shader
	PixelShader* CookTorrance = new PixelShader(*shc, SH_BASE_PATH"CookTorrancePS.hlsl", { regularSD }, { lightBufferDesc });
	CookTorrance->describeBuffers({ lightBufferMeta });
	addPixShader("CookTorrancePS", CookTorrance);

	// CSM Scene shader
	PixelShader* csmScenePs = new PixelShader(*shc, SH_BASE_PATH"csmScenePS.hlsl", { regularSD }, { });
	//csmScenePs->describeBuffers({ lightBufferMeta });
	addPixShader("csmScenePS", csmScenePs);

	PixelShader* clusterDebugPs = new PixelShader(*shc, SH_BASE_PATH"clusterDebug.hlsl", { regularSD }, {});
	addPixShader("clusterDebugPS", clusterDebugPs);

	PixelShader* clusterPs = new PixelShader(*shc, SH_BASE_PATH"ClusteredPS.hlsl", { regularSD }, {});
	addPixShader("clusterPS", clusterPs);

	PixelShader* skyboxTrianglePS = new PixelShader(*shc, SH_BASE_PATH"SkyboxTrianglePS.hlsl", { skbyoxSD }, {});
	addPixShader("skyboxTrianglePS", skyboxTrianglePS);

#undef SH_BASE_PATH
}
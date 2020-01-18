#include "ShaderCache.h"
#include "ShaderDataStructs.h"
#include "Math.h"



void ShaderCache::init(ShaderCompiler* shCompiler)
{
	_shCompiler = shCompiler;
}



void ShaderCache::createAllShadersBecauseIAmTooLazyToMakeThisDataDriven()
{
	//VERTEX SHADERS

	//DATA
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc = ShaderCompiler::createCBufferDesc(sizeof(WMBuffer));

	std::vector<D3D11_INPUT_ELEMENT_DESC> inLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D11_SAMPLER_DESC sbSamplerDesc = _shCompiler->createSamplerDesc();

	//CREATION
	VertexShader* vs = new VertexShader(*_shCompiler, L"lightVS.hlsl", inLayout, { matrixBufferDesc });
	//make this happen during construction, if possible...need to load from files tbh
	CBufferMeta meta(0, sizeof(SMatrix));
	meta._fields.push_back(CBufferFieldDesc(CBUFFER_FIELD_CONTENT::TRANSFORM, 0, sizeof(SMatrix)));
	vs->describeBuffers({ meta });

	addVertShader("basicVS", vs);










	//PIXEL SHADERS

	//DATA
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc = ShaderCompiler::createCBufferDesc(sizeof(LightBuffer));

	//CREATION
	PixelShader* ps = new PixelShader(*_shCompiler, L"lightPS.hlsl", sbSamplerDesc, { lightBufferDesc });
	CBufferMeta psmeta(0, sizeof(LightBuffer));
	psmeta._fields.push_back(CBufferFieldDesc(CBUFFER_FIELD_CONTENT::P_LIGHT, 0, sizeof(LightBuffer)));
	ps->describeBuffers({ psmeta });

	addPixShader("lightPS", ps);
}



VertexShader* ShaderCache::getVertShader(const std::string& name)
{
	std::unordered_map<std::string, VertexShader*>::const_iterator found = _vsMap.find(name);
	if (found == _vsMap.end())
		return nullptr;
	else
		return found->second;
}



PixelShader* ShaderCache::getPixShader(const std::string& name)
{
	std::unordered_map<std::string, PixelShader*>::const_iterator found = _psMap.find(name);
	if (found == _psMap.end())
		return nullptr;
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
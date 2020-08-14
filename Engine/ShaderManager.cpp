#include "ShaderManager.h"



void ShaderManager::loadExistingKeys(const std::wstring& path)
{
	auto shaderFiles = FileUtils::getFilesByExt(path, ".cmp");

	for (auto& file : shaderFiles)
	{
		const std::string& filename = file.path().filename().string();

		UINT division = filename.find('.');

		uint64_t key = std::stoull(filename.substr(0, division));

		_existingShaders.insert({ key, ShaderPack{ nullptr, nullptr } });
	}
}



ShaderPack* ShaderManager::getShaderAuto(VertSignature vertSig, Material* mat, SHG_LIGHT_MODEL lightModel)
{
	ShaderKey shaderKey = ShaderGenerator::CreateShaderKey(vertSig, mat, lightModel);

	ShaderPack* shPack = getShaderByKey(shaderKey);

	if (shPack)
		return shPack;

	ShaderPack newPack = CreateShader(_pDevice, shaderKey, vertSig, mat);

	auto inserted = _existingShaders.insert({ shaderKey, newPack });

	return &(inserted.first->second);
}



ShaderPack* ShaderManager::getShaderByKey(ShaderKey shaderKey)
{
	// Check if exists, check if loaded, create/load if needed, return
	auto it = _existingShaders.find(shaderKey);

	if (it != _existingShaders.end())	// Exists
	{
		ShaderPack& sp = it->second;
		if (sp.vs == nullptr)	// VS not loaded, load
			sp.vs = static_cast<VertexShader*>(loadFromKey(shaderKey, L"vs.cmp"));
		if (!sp.ps)				// PS not loaded, load
			sp.ps = static_cast<PixelShader*>(loadFromKey(shaderKey, L"ps.cmp"));

		return &it->second;
	}

	// Does not exist, create and add to existing
	return nullptr;
}



Shader* ShaderManager::loadFromKey(ShaderKey shaderKey, const wchar_t* ext)
{
	std::wstring wFileName = NATURAL_COMPS + std::to_wstring(shaderKey) + ext;

	// Load shaders, can't work yet...
	if (ext == L"vs.cmp")
	{
		return loadVertexShader(wFileName, shaderKey);
	}
	if (ext == L"ps.cmp")
	{
		return loadPixelShader(wFileName, shaderKey);
	}
	return nullptr;
}



ShaderPack ShaderManager::CreateShader(ID3D11Device* device, uint64_t shaderKey, VertSignature vertSig, Material* mat)
{
	ShaderGenerator::CreatePermFromKey(ShaderGenerator::AllOptions, shaderKey);

	ShaderCompiler shc;
	shc.init(device);

	// VS
	std::wstring vsPathW(NATURAL_PERMS + std::to_wstring(shaderKey) + L"vs.hlsl");
	std::wstring cmpVsPath(NATURAL_COMPS + std::to_wstring(shaderKey) + L"vs.cmp");

	ID3DBlob* vsBlob = shc.compileToBlob(vsPathW, "vs_5_0");
	ID3D11VertexShader* d3dvs = shc.blobToVS(vsBlob);

	auto vertInLayElements = vertSig.createVertInLayElements();
	D3D11_BUFFER_DESC WMBufferDesc = CBuffer::createDesc(sizeof(SMatrix));
	CBufferMeta WMBufferMeta(0, WMBufferDesc.ByteWidth);
	WMBufferMeta.addFieldDescription(CBUFFER_FIELD_CONTENT::TRANSFORM, 0, sizeof(SMatrix));

	PersistVertexShader(cmpVsPath.c_str(), vsBlob, vertSig, { WMBufferDesc });

	VertexShader* vs = new VertexShader(device, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		vsPathW, vertInLayElements, { WMBufferDesc });
	vs->describeBuffers({ WMBufferMeta });
	vsBlob->Release();

	// PS
	std::wstring psPathW(NATURAL_PERMS + std::to_wstring(shaderKey) + L"ps.hlsl");
	std::wstring cmpPsPath(NATURAL_COMPS + std::to_wstring(shaderKey) + L"ps.cmp");

	ID3DBlob* psBlob = shc.compileToBlob(psPathW, "ps_5_0");
	ID3D11PixelShader* d3dps = shc.blobToPS(psBlob);

	auto samplerDescriptions = mat->createSamplerDescs();

	PixelShader* ps = new PixelShader(device, psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
		psPathW, samplerDescriptions, {});

	PersistPixelShader(cmpPsPath.c_str(), psBlob, samplerDescriptions, {});
	psBlob->Release();

	return ShaderPack{ vs, ps };
}
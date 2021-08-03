#include "pch.h"

#include "ShaderManager.h"
#include "Material.h"


void ShaderManager::loadExistingKeys(const std::wstring& path)
{
	auto shaderFiles = FileUtils::GetFilesByExt(path, ".cmp");

	for (auto& file : shaderFiles)
	{
		const std::string& filename = file.path().filename().string();

		auto division = filename.find('.');

		ShaderGenKey key = std::stoull(filename.substr(0, division));

		_existingShaders.insert({ key, ShaderPack{ nullptr, nullptr } });
	}
}


ShaderPack* ShaderManager::getBestFit(VertSignature vertSig, Material* mat, SHG_LIGHT_MODEL lightModel)
{
	ShaderGenKey shaderKey = ShaderGenerator::CreateShaderKey(vertSig, mat, lightModel);

	ShaderPack* shPack = getShaderByKey(shaderKey);

	if (shPack)
		return shPack;

	ShaderPack newPack = CreateShader(_pDevice, shaderKey, vertSig, mat);

	auto inserted = _existingShaders.insert({ shaderKey, newPack });

	return &(inserted.first->second);
}


ShaderPack* ShaderManager::getShaderByKey(ShaderGenKey shaderKey)
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


Shader* ShaderManager::loadFromKey(ShaderGenKey shaderKey, const wchar_t* ext)
{
	std::wstring wFileName = COMPILED_FOLDER + std::to_wstring(shaderKey) + ext;

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


// @TODO this has way too many implementation details of ShaderGenerator, shouldn't be here.
ShaderPack ShaderManager::CreateShader(ID3D11Device* device, uint64_t shaderKey, VertSignature vertSig, Material* mat)
{
	ShaderGenerator::CreatePermFromKey(ShaderGenerator::AllOptions, shaderKey);

	ShaderCompiler shc(device);

	// VS
	std::wstring vsPathW(PERMUTATIONS_FOLDER + std::to_wstring(shaderKey) + L"vs.hlsl");
	std::wstring cmpVsPath(COMPILED_FOLDER + std::to_wstring(shaderKey) + L"vs.cmp");

	ID3DBlob* vsBlob = shc.compileToBlob(vsPathW, "vs_5_0");
	ID3D11VertexShader* d3dvs = shc.blobToVS(vsBlob);

	auto vertInLayElements = vertSig.createVertInLayElements();

	D3D11_BUFFER_DESC WMBufferDesc = CBuffer::createDesc(sizeof(SMatrix));
	CBufferMeta WMBufferMeta(0, WMBufferDesc.ByteWidth);
	WMBufferMeta.addFieldDescription(0, sizeof(SMatrix));

	PersistVertexShader(cmpVsPath.c_str(), vsBlob, vertSig, { WMBufferDesc });

	VertexShader* vs = new VertexShader(device, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		vsPathW, vertInLayElements, { WMBufferDesc });
	vs->describeBuffers({ WMBufferMeta });
	vsBlob->Release();

	// PS
	std::wstring psPathW(PERMUTATIONS_FOLDER + std::to_wstring(shaderKey) + L"ps.hlsl");
	std::wstring cmpPsPath(COMPILED_FOLDER + std::to_wstring(shaderKey) + L"ps.cmp");

	ID3DBlob* psBlob = shc.compileToBlob(psPathW, "ps_5_0");
	ID3D11PixelShader* d3dps = shc.blobToPS(psBlob);

	auto samplerDescriptions = mat->createSamplerDescs();

	PixelShader* ps = new PixelShader(device, psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
		psPathW, samplerDescriptions, {});

	PersistPixelShader(cmpPsPath.c_str(), psBlob, samplerDescriptions, {});
	psBlob->Release();

	return ShaderPack{ vs, ps };
}


// @TODO This should be in an editor wrapper, separate concerns
void ShaderManager::DisplayShaderPicker(VertSignature vertSig, Material* mat, ID3D11Device* device)
{
	// For UI
	struct LightModelIndex
	{
		std::string name;
		SHG_LIGHT_MODEL index;
	};
	uint64_t shaderKey{ 0 };

	static const std::vector<LightModelIndex> lmiOptions
	{
		{ "NONE",		SHG_LM_NONE},
		{ "LAMBERT",	SHG_LM_LAMBERT},
		{ "PHONG",		SHG_LM_PHONG}
	};

	// Default lambert
	static const LightModelIndex* selected = &lmiOptions[1];

	if (ImGui::Begin("Shader picker"))
	{
		if (ImGui::BeginCombo("Light model", selected->name.data()))
		{
			for (UINT i = 0; i < lmiOptions.size(); ++i)
			{
				if (ImGui::Selectable(lmiOptions[i].name.c_str()))
					selected = &lmiOptions[i];
			}
			ImGui::EndCombo();
		}
	}

	shaderKey = ShaderGenerator::CreateShaderKey(vertSig, mat, selected->index);

	ImGui::Text("Key: %lu", shaderKey);

	if (ImGui::Button("Create shader"))
	{
		CreateShader(device, shaderKey, vertSig, mat);
	}

	ImGui::End();
}
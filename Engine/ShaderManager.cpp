#include "pch.h"

#include "ShaderManager.h"
#include "Material.h"
#include "IMGUI/imgui.h"


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


ShaderPack* ShaderManager::getBestFit(VertSignature vertSig, Material* mat, ShGen::LIGHT_MODEL lightModel)
{
	ShaderGenKey shaderKey = ShGen::ShaderGenerator::CreateShaderKey(vertSig, mat, lightModel);

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
		if (!sp.vs)				// VS not loaded, load
			sp.vs = std::static_pointer_cast<VertexShader>(loadFromKey(shaderKey, L"vs.cmp"));
		if (!sp.ps)				// PS not loaded, load
			sp.ps = std::static_pointer_cast<PixelShader>(loadFromKey(shaderKey, L"ps.cmp"));

		return &it->second;
	}

	// Does not exist, create and add to existing
	return nullptr;
}


std::shared_ptr<Shader> ShaderManager::loadFromKey(ShaderGenKey shaderKey, const wchar_t* ext)
{
	std::wstring wFileName = ShGen::COMPILED_FOLDER + std::to_wstring(shaderKey) + ext;

	if (ext == L"vs.cmp")
	{
		return std::make_shared<VertexShader>(LoadVertexShader(wFileName, _pDevice));
	}
	if (ext == L"ps.cmp")
	{
		return std::make_shared<PixelShader>(LoadPixelShader(wFileName, _pDevice));
	}
	return nullptr;
}


// @TODO this has way too many implementation details of ShaderGenerator, shouldn't be here.
ShaderPack ShaderManager::CreateShader(ID3D11Device* device, uint64_t shaderKey, VertSignature vertSig, Material* mat)
{
	ShGen::ShaderGenerator::CreatePermFromKey(ShGen::ShaderGenerator::AllOptions, shaderKey);

	ShaderCompiler shc(device);

	std::shared_ptr<VertexShader> vs{};
	std::shared_ptr<PixelShader> ps{};

	// VS
	std::wstring vsPathW(ShGen::PERMUTATIONS_FOLDER + std::to_wstring(shaderKey) + L"vs.hlsl");
	std::wstring cmpVsPath(ShGen::COMPILED_FOLDER + std::to_wstring(shaderKey) + L"vs.cmp");

	{
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = shc.compileToBlob(vsPathW, "vs_5_0");

		auto vertInLayElements = vertSig.createVertInLayElements();

		D3D11_BUFFER_DESC WMBufferDesc = CBuffer::createDesc(sizeof(SMatrix));
		CBufferMeta WMBufferMeta(0, WMBufferDesc.ByteWidth);
		WMBufferMeta.addFieldDescription(0, sizeof(SMatrix));

		PersistVertexShader(cmpVsPath.c_str(), vsBlob.Get(), vertSig, { WMBufferDesc });

		vs = std::make_shared<VertexShader>(VertexShader(device, vsBlob->GetBufferPointer(), static_cast<uint32_t>(vsBlob->GetBufferSize()),
			vsPathW, vertInLayElements, { WMBufferDesc }));
		vs->describeBuffers({ WMBufferMeta });
	}

	// PS
	{
		std::wstring psPathW(ShGen::PERMUTATIONS_FOLDER + std::to_wstring(shaderKey) + L"ps.hlsl");
		std::wstring cmpPsPath(ShGen::COMPILED_FOLDER + std::to_wstring(shaderKey) + L"ps.cmp");

		Microsoft::WRL::ComPtr<ID3DBlob> psBlob = shc.compileToBlob(psPathW, "ps_5_0");

		auto samplerDescriptions = mat->createSamplerDescs();

		ps = std::make_shared<PixelShader>(PixelShader(device, psBlob->GetBufferPointer(), static_cast<uint32_t>(psBlob->GetBufferSize()),
			psPathW, samplerDescriptions, {}));

		PersistPixelShader(cmpPsPath.c_str(), psBlob.Get(), samplerDescriptions, {});
	}

	return ShaderPack{ vs, ps };
}
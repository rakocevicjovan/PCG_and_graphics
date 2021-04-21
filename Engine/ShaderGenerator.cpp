#include "pch.h"
#include "ShaderGenerator.h"
#include "Material.h"



void ShaderGenerator::AddToKey(const VertSignature& vertSig, uint64_t& key, 
	VAttribSemantic semantic, const ShaderOption& shOpt)
{
	UINT elemCount = vertSig.countAttribute(semantic);
	elemCount = min(elemCount, shOpt._maxVal);
	key |= (static_cast<uint64_t>(elemCount) << shOpt._offset);
}



void ShaderGenerator::EncodeVertexData(const VertSignature& vertSig, uint64_t& key)
{
	AddToKey(vertSig, key, VAttribSemantic::TEX_COORD, SHG_OPT_TEX);
	AddToKey(vertSig, key, VAttribSemantic::COL, SHG_OPT_COL);
	AddToKey(vertSig, key, VAttribSemantic::NORMAL, SHG_OPT_NRM);
	AddToKey(vertSig, key, VAttribSemantic::TANGENT, SHG_OPT_TAN);
	AddToKey(vertSig, key, VAttribSemantic::BITANGENT, SHG_OPT_BTN);
	AddToKey(vertSig, key, VAttribSemantic::B_IDX, SHG_OPT_SIW);
}



void ShaderGenerator::EncodeTextureData(const std::vector<MaterialTexture>& texData, uint64_t& key)
{
	for (const auto& [metaData, tex] : texData)
	{
		if (!tex)	// Don't build a shader for non-existent textures
			continue;

		auto iter = TEX_ROLE_TO_SHADER_OPTION.find(metaData._role);
		if (iter != TEX_ROLE_TO_SHADER_OPTION.end())
			key |= (1Ui64 << iter->second->_offset);
		else
			OutputDebugStringA("Matching shader option for texture type not found.");
	}
}



ShaderKey ShaderGenerator::CreateShaderKey(const VertSignature& vertSig, const Material* mat, UINT lmIndex)
{
	ShaderKey shaderKey{0ul};

	shaderKey |= (static_cast<uint64_t>(lmIndex) << SHG_OPT_LMOD._offset);
	EncodeVertexData(vertSig, shaderKey);
	EncodeTextureData(mat->_materialTextures, shaderKey);

	return shaderKey;
}



void ShaderGenerator::CreatePermFromKey(const std::vector<ShaderOption>& options, uint64_t key)
{
	// Consider keeping in memory, would improve performance a lot
	ID3DBlob* protoVSbuffer = nullptr;
	ID3DBlob* protoPSbuffer = nullptr;

	if (FAILED(D3DReadFileToBlob(VS_PROTOSHADER, &protoVSbuffer)) || 
		FAILED(D3DReadFileToBlob(PS_PROTOSHADER, &protoPSbuffer)))
	{
		OutputDebugStringA("Shader prototype(s) not accessible.");
		exit(2001);
	}

	std::list<std::string> values;
	uint64_t total = 0ul;
	auto matchingPermOptions = ParseKeyToOptions(options, key, values, total);

	CreateShPerm(NATURAL_PERMS, protoVSbuffer, matchingPermOptions, L"vs", total);
	CreateShPerm(NATURAL_PERMS, protoPSbuffer, matchingPermOptions, L"ps", total);

	protoVSbuffer->Release();
	protoPSbuffer->Release();
}



inline std::vector<D3D_SHADER_MACRO> ShaderGenerator::ParseKeyToOptions(
	const std::vector<ShaderOption>& options, uint64_t key, std::list<std::string>& values, uint64_t& total)
{
	UINT optionCount = options.size();

	std::vector<D3D_SHADER_MACRO> matchedOptions;
	matchedOptions.reserve(optionCount);

	std::string permOptDebugString;
	permOptDebugString.reserve(100);

	permOptDebugString = std::to_string(key) + " POS ";

	for (UINT j = 0; j < optionCount; ++j)
	{
		const ShaderOption& so = options[j];
		uint64_t result = (key >> so._offset) &  (~(~0u << so._numBits));

		// If current option fits the bitmask, add it in
		bool hasDependency{ so.depMask != (~0u) };
		bool dependency = (!hasDependency) ? true : ((so.depMask & key) == so.depMask);

		if (result > 0 && result <= so._maxVal && dependency)
		{
			values.push_back(std::to_string(result));
			matchedOptions.push_back({ so.name, values.back().c_str() });
			total += (result << so._offset);
			permOptDebugString += so.name + std::to_string(result) + " ";
		}
	}

	matchedOptions.push_back({ NULL, NULL });	// Required by d3d api

	permOptDebugString += "\n";
	OutputDebugStringA(permOptDebugString.c_str());

	return matchedOptions;
}



void ShaderGenerator::CreateShPerm(const std::wstring& outDirPath, ID3DBlob* textBuffer,
	const std::vector<D3D_SHADER_MACRO>& permOptions, const wchar_t* type, uint64_t total)
{
	HRESULT res;
	ID3DBlob* preprocessedBuffer = nullptr;
	ID3DBlob* errorMessage = nullptr;

	res = D3DPreprocess(textBuffer->GetBufferPointer(), textBuffer->GetBufferSize(),
		nullptr, permOptions.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE,
		&preprocessedBuffer, &errorMessage);

	if (FAILED(res))
	{
		OutputDebugStringA(reinterpret_cast<const char*>(errorMessage->GetBufferPointer()));
	}

	std::wstring finalFileName = outDirPath + std::to_wstring(total) + type + L".hlsl";

	FileUtils::writeAllBytes(finalFileName.c_str(),
		preprocessedBuffer->GetBufferPointer(),
		preprocessedBuffer->GetBufferSize());

	if (preprocessedBuffer)
		preprocessedBuffer->Release();

	if (errorMessage)
		errorMessage->Release();

	sizeof(ShaderOption);
}



bool ShaderGenerator::preprocessAllPermutations(const std::wstring& ogFilePathW, const std::wstring& outDirPath)
{
	std::set<uint64_t> existingKeys;

	ID3DBlob* textBuffer = nullptr;

	if (FAILED(D3DReadFileToBlob(ogFilePathW.c_str(), &textBuffer)))
	{
		OutputDebugStringA("Shader prototype not accessible.");
		exit(2001);
	}

	const std::vector<ShaderOption>& options = AllOptions;

	UINT optionCount = options.size();
	UINT bitCount = 0u;

	for (UINT i = 0; i < options.size(); ++i)
		bitCount += options[i]._numBits;

	for (uint64_t i = 0; i < (1Ui64 << bitCount); ++i)
	{
		CreatePermFromKey(outDirPath, textBuffer, options, i, existingKeys);
	}

	if (textBuffer)
	{
		textBuffer->Release();
		textBuffer = nullptr;
	}

	return true;
}



void ShaderGenerator::CreatePermFromKey(
	const std::wstring& outDirPath,
	ID3DBlob*& textBuffer,
	const std::vector<ShaderOption>& options,
	uint64_t key,
	std::set<uint64_t>& existingKeys)
{
	std::list<std::string> values;
	uint64_t total = 0ul;
	auto matchingPermOptions = ParseKeyToOptions(options, key, values, total);

	if (!existingKeys.insert(total).second)
		return;

	CreateShPerm(outDirPath.c_str(), textBuffer, matchingPermOptions, L"vs", total);
}



const std::vector<ShaderOption> ShaderGenerator::AllOptions
{
	SHG_OPT_TEX, SHG_OPT_NRM, SHG_OPT_COL, SHG_OPT_TAN,
	SHG_OPT_BTN, SHG_OPT_SIW, SHG_OPT_INS, 
	// Pixel shader options
	SHG_OPT_LMOD, SHG_OPT_ALPHA, SHG_OPT_FOG, SHG_OPT_SHD, SHG_OPT_GAMMA,
	// Texture options
	SHG_TX_DIF, SHG_TX_NRM, SHG_TX_SPC, SHG_TX_SHN, SHG_TX_OPC,
	SHG_TX_DPM, SHG_TX_AMB, SHG_TX_MTL, SHG_TX_RGH, SHG_TX_RFL, SHG_TX_RFR
};
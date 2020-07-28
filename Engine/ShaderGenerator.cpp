#pragma once
#include "ShaderGenerator.h"



bool ShaderGenerator::preprocessAllPermutations(const std::wstring& ogFilePathW, const std::string& outDirPath)
{
	std::set<uint64_t> existingKeys;

	ID3DBlob* textBuffer = nullptr;

	if (FAILED(D3DReadFileToBlob(ogFilePathW.c_str(), &textBuffer)))
	{
		OutputDebugStringA("Shader prototype not accessible.");
		exit(2001);
	}

	std::vector<ShaderOption> options = getVsOptions();

	UINT optionCount = options.size();
	UINT bitCount = 0u;

	for (UINT i = 0; i < options.size(); ++i)
		bitCount += options[i]._numBits;

	std::vector<D3D_SHADER_MACRO> matchingPermOptions;
	matchingPermOptions.reserve(optionCount);

	std::string debugString;
	debugString.reserve(100);

	UINT counter = 0u;

	for (uint64_t i = 0; i < (1 << bitCount); ++i)
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
	const std::string& outDirPath,
	ID3DBlob*& textBuffer,
	const std::vector<ShaderOption>& options,
	uint64_t key,
	std::set<uint64_t>& existingKeys)
{
	UINT optionCount = options.size();
	uint64_t total = 0;

	std::vector<D3D_SHADER_MACRO> matchingPermOptions;
	matchingPermOptions.reserve(optionCount);

	std::string permOptDebugString;
	permOptDebugString.reserve(100);

	permOptDebugString = std::to_string(key) + " POS ";

	std::list<std::string> valStrings;

	for (UINT j = 0; j < optionCount; ++j)
	{
		const ShaderOption& so = options[j];

		uint64_t shifted = key >> so._offset;
		uint64_t bitMask = (~(~0u << so._numBits));
		uint64_t result = shifted & bitMask;

		// If current option fits the bitmask, add it in
		bool hasDependency{ so.depMask != (~0u) };

		bool dependency = (!hasDependency) ?
			true : ((so.depMask & key) == so.depMask);

		if (result > 0 && result <= so._maxVal && dependency)
		{
			valStrings.push_back(std::to_string(result));
			D3D_SHADER_MACRO d3dshm{ so.name.c_str(), valStrings.back().c_str() };
			matchingPermOptions.push_back(d3dshm);
			permOptDebugString += so.name + std::to_string(result);
			permOptDebugString += " ";
			total += (result << so._offset);
		}
	}

	if (!existingKeys.insert(total).second)
	{
		return;
	}

	matchingPermOptions.push_back({ NULL, NULL });	// Required by d3d api

	createShPerm(outDirPath.c_str(), textBuffer, matchingPermOptions, total);

	permOptDebugString += "\n";
	OutputDebugStringA(permOptDebugString.c_str());

	return;
}



void ShaderGenerator::createShPerm(const std::string& outDirPath, ID3DBlob* textBuffer,
	const std::vector<D3D_SHADER_MACRO>& permOptions, uint64_t total)
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

	std::string finalFileName = outDirPath + std::to_string(total) + ".hlsl";

	FileUtils::writeAllBytes(finalFileName.c_str(),
		preprocessedBuffer->GetBufferPointer(),
		preprocessedBuffer->GetBufferSize());

	if (preprocessedBuffer)
	{
		preprocessedBuffer->Release();
		preprocessedBuffer = nullptr;
	}

	if (errorMessage)
	{
		errorMessage->Release();
		errorMessage = nullptr;
	}
}
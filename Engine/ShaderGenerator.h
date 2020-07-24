#pragma once
#include "ShaderCompiler.h"
#include "FileUtilities.h"
#include <algorithm>
#include <set>



#pragma pack(show)
#pragma pack(push, 1)
enum SHG_VS_SETTINGS : uint32_t
{
	SHG_VS_TEXCOORDS	= (1 << 0),
	SHG_VS_NORMALS		= (1 << 1),
	SHG_VS_COLOUR		= (1 << 2),
	SHG_VS_TANGENT		= (1 << 3 | SHG_VS_NORMALS),
	SHG_VS_BITANGENT	= (1 << 4 | SHG_VS_NORMALS | SHG_VS_TANGENT),
	SHG_VS_SKINIDXWGT	= (1 << 5),
	SHG_VS_INSTANCING	= (1 << 7),
	SHG_VS_WORLD_POS	= (1 << 8)
};

enum SHG_PS_SETTINGS : uint32_t
{
	SHG_PS_LIGHTMODEL	= (1 << 0),
	SHG_PS_ALPHA		= (1 << 1),
	SHG_PS_NORMALMAP	= (1 << 2),
	SHG_PS_FOG			= (1 << 3),
	SHG_PS_SHADOW		= (1 << 4),
	SHG_PS_GAMMA		= (1 << 5)
};
#pragma pack(pop)



class ShaderGenerator
{
	struct ShaderOption
	{
		D3D_SHADER_MACRO _macro;
		uint64_t _bitmask;
	};

public:

	static std::vector<ShaderOption> getVsOptions()
	{
		// HLSL preprocessor only checks for macro name, and not the value?
		return
		{ 
			{{ "TEX", "true" }, SHG_VS_SETTINGS::SHG_VS_TEXCOORDS	},
			{{ "NRM", "true" }, SHG_VS_SETTINGS::SHG_VS_NORMALS		},
			{{ "COL", "true" }, SHG_VS_SETTINGS::SHG_VS_COLOUR		},
			{{ "TAN", "true" }, SHG_VS_SETTINGS::SHG_VS_TANGENT		},
			{{ "BTN", "true" }, SHG_VS_SETTINGS::SHG_VS_BITANGENT	},
			{{ "SIW", "true" }, SHG_VS_SETTINGS::SHG_VS_SKINIDXWGT	},
			{{ "INS", "true" }, SHG_VS_SETTINGS::SHG_VS_INSTANCING	},
			{{ "WPS", "true" }, SHG_VS_SETTINGS::SHG_VS_WORLD_POS	}
		};
	}



	static std::vector<ShaderOption> getPsOptions()
	{
	}



	static void createShPerm(ID3DBlob* textBuffer, const std::vector<D3D_SHADER_MACRO>& permOptions, uint64_t total)
	{
		HRESULT res;
		ID3DBlob* preprocessedBuffer = nullptr;
		ID3DBlob* errorMessage = nullptr;

		res = D3DPreprocess(textBuffer->GetBufferPointer(), textBuffer->GetBufferSize(),
			nullptr, permOptions.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE,
			&preprocessedBuffer, &errorMessage);

		std::string finalFileName = "ShGen\\GeneratedVS\\vs_" + std::to_string(total) + ".hlsl";

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



	static bool preprocessAllPermutations(const std::wstring& filePathW, std::vector<ShaderOption>& optionSet)	
	{
		std::set<uint64_t> _existing;
		std::string filePath = "ShGen\\VS_proto.hlsl";

		ID3DBlob* textBuffer = nullptr;
		
		if (FAILED(D3DReadFileToBlob(filePathW.c_str(), &textBuffer)))
		{
			OutputDebugStringA("Couldn't read shader template file.");
			exit(2001);
		}

		UINT optionCount = optionSet.size();

		std::vector<D3D_SHADER_MACRO> matchingPermOptions;
		matchingPermOptions.reserve(optionCount);

		std::string permOptDebugString;
		permOptDebugString.reserve(50);

		UINT counter = 0u;

		for (uint64_t i = 0; i < ( 1 << (optionCount) ); ++i)
		{
			uint64_t total = 0;
			permOptDebugString = std::to_string(++counter) + ": POS ";

			// Iterate through all options and add macros of those that are in this permutation to
			// the list of the macros passed to the shader compiler
			for (UINT j = 0; j < optionCount; ++j)
			{
				uint64_t currentOptionBitmask = optionSet[j]._bitmask;
				uint64_t andResult = currentOptionBitmask & i;

				// If current option fits the bitmask, add it in
				if (andResult == currentOptionBitmask)
				{
					matchingPermOptions.push_back(optionSet[j]._macro);
					permOptDebugString += optionSet[j]._macro.Name;
					permOptDebugString += " ";

					total += andResult;
				}
			}

			// I barely remember how this works but it should eliminate doubles?
			if (!_existing.insert(total).second)
			{
				matchingPermOptions.clear();
				permOptDebugString.clear();
				continue;
			}

			matchingPermOptions.push_back({ NULL, NULL });	// Required by d3d api

			createShPerm(textBuffer, matchingPermOptions, total);

			permOptDebugString += "\n";
			OutputDebugStringA(permOptDebugString.c_str());	

			matchingPermOptions.clear();
			permOptDebugString.clear();
		}

		if (textBuffer)
		{
			textBuffer->Release();
			textBuffer = nullptr;
		}

		return true;
	}
};


// Although these can be separate input options, there is no need for that. 
// If the shader requires tex coordinate output, it must have them (generally speaking) as input anyways...
// therefore we save on the number of permutations - same for normals and tangents
//D3D_SHADER_MACRO i_tex = { "TEX", "true" };	
//D3D_SHADER_MACRO i_nrm = { "NRM", "true" };
//D3D_SHADER_MACRO i_tan = { "TAN", "true" };
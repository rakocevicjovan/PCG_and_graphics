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

	struct Option
	{
		std::string name;
		//std::vector<std::string> values;
		UINT _offset;
		UINT _numBits = 1;
	};

public:

	static std::vector<ShaderOption> getVsOptions()
	{
		// HLSL preprocessor only checks for macro name, and not the value?
		return
		{ 
			{{ "TEX", "" }, SHG_VS_TEXCOORDS	},
			{{ "NRM", "" }, SHG_VS_NORMALS		},
			{{ "COL", "" }, SHG_VS_COLOUR		},
			{{ "TAN", "" }, SHG_VS_TANGENT		},
			{{ "BTN", "" }, SHG_VS_BITANGENT	},
			{{ "SIW", "" }, SHG_VS_SKINIDXWGT	},
			{{ "INS", "" }, SHG_VS_INSTANCING	},
			{{ "WPS", "" }, SHG_VS_WORLD_POS	}
		};
	}



	static std::vector<ShaderOption> getPsOptions()
	{
		return
		{
			{{ "", "" }, SHG_PS_LIGHTMODEL },
			{{ "", "" }, SHG_PS_ALPHA },
			{{ "", "" }, SHG_PS_NORMALMAP},
			{{ "", "" }, SHG_PS_FOG},
			{{ "", "" }, SHG_PS_SHADOW},
			{{ "", "" }, SHG_PS_GAMMA}
		};
	}


	static bool preprocessAllPermutations(const std::wstring& filePathW, std::vector<ShaderOption>& optionSet)	
	{
		std::set<uint64_t> _existing;

		ID3DBlob* textBuffer = nullptr;
		
		if (FAILED(D3DReadFileToBlob(filePathW.c_str(), &textBuffer)))
		{
			OutputDebugStringA("Couldn't read shader template file.");
			exit(2001);
		}

		std::vector<Option> options
		{
			{"TEX", 0	},
			{"NRM", 1	},
			{"COL", 2	},
			{"TAN", 3	},
			{"BTN", 4	},
			{"SIW", 5	},
			{"INS", 6	},
			{"WPS", 7	}
		};

		UINT optionCount = optionSet.size();

		std::vector<D3D_SHADER_MACRO> matchingPermOptions;
		matchingPermOptions.reserve(optionCount);

		std::string debugString;
		debugString.reserve(100);

		UINT counter = 0u;

		for (uint64_t i = 0; i < ( 1 << optionCount); ++i)
		{
			uint64_t total = CreatePermFromKey(i, options, textBuffer);
			/*
			uint64_t total = 0;
			debugString = std::to_string(++counter) + ": POS ";

			// Iterate through all options and add suitable macros to 
			// the list of the macros passed to the shader compiler
			for (UINT j = 0; j < optionCount; ++j)
			{
				uint64_t requestedOption = optionSet[j]._bitmask;
				uint64_t andResult = requestedOption & i;

				// If current option fits the bitmask, add it in
				if (andResult == requestedOption)
				{
					matchingPermOptions.push_back(optionSet[j]._macro);
					debugString += optionSet[j]._macro.Name;
					debugString += " ";

					total += andResult;
				}
			}

			// I barely remember how this works but it should eliminate doubles?
			if (!_existing.insert(total).second)
			{
				matchingPermOptions.clear();
				debugString.clear();
				continue;
			}

			matchingPermOptions.push_back({ NULL, NULL });	// Required by d3d api

			createShPerm(textBuffer, matchingPermOptions, total);

			debugString += "\n";
			OutputDebugStringA(debugString.c_str());	

			matchingPermOptions.clear();
			debugString.clear();
			*/
		}

		if (textBuffer)
		{
			textBuffer->Release();
			textBuffer = nullptr;
		}

		return true;
	}



	static uint64_t CreatePermFromKey(uint64_t key, std::vector<Option>& options, ID3DBlob*& textBuffer)
	{
		UINT optionCount = options.size();
		uint64_t total = 0;

		std::vector<D3D_SHADER_MACRO> matchingPermOptions;
		matchingPermOptions.reserve(optionCount);

		std::string permOptDebugString;
		permOptDebugString.reserve(100);

		permOptDebugString = "POS ";

		for (UINT j = 0; j < optionCount; ++j)
		{
			Option& o = options[j];

			uint32_t shifted = key >> o._offset;
			uint32_t bitMask = (~(~0u << o._numBits));

			uint32_t result = shifted & bitMask;

			// If current option fits the bitmask, add it in
			if (result > 0)
			{
				D3D_SHADER_MACRO d3dshm{ o.name.c_str(), std::to_string(result).c_str() };
				matchingPermOptions.push_back(d3dshm);
				permOptDebugString += o.name;
				permOptDebugString += " ";
			}
			total += (result << o._offset);
		}

		matchingPermOptions.push_back({ NULL, NULL });	// Required by d3d api

		createShPerm(textBuffer, matchingPermOptions, total);

		permOptDebugString += "\n";
		OutputDebugStringA(permOptDebugString.c_str());

		return total;
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
};
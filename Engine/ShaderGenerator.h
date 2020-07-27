#pragma once
#include "ShaderCompiler.h"
#include "FileUtilities.h"
#include <algorithm>
#include <set>
#include <string>



#pragma pack(show)
#pragma pack(push, 1)
enum SHG_VS_SETTINGS : uint32_t
{
	SHG_VS_TEXCOORDS	= (1 << 0 | 1 << 1 | 1<< 2),
	SHG_VS_NORMALS		= (1 << 3),
	/*
	SHG_VS_COLOUR		= (1 << 2),
	SHG_VS_TANGENT		= (1 << 3 | SHG_VS_NORMALS),
	SHG_VS_BITANGENT	= (1 << 4 | SHG_VS_NORMALS | SHG_VS_TANGENT),
	SHG_VS_SKINIDXWGT	= (1 << 5),
	SHG_VS_INSTANCING	= (1 << 7),
	SHG_VS_WORLD_POS	= (1 << 8)
	*/
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
	/*struct ShaderOption
	{
		std::string name;
		uint64_t _bitmask;
	};*/

	struct ShaderOption
	{
		std::string name;
		uint16_t _offset;
		uint16_t _numBits = 1;
		uint16_t _maxVal = 1;
		uint64_t depMask = (~0ul);
	};

public:

	static std::vector<ShaderOption> getVsOptions()
	{
		return
		{ 
			{"TEX", 0, 3, 4},
			{"NRM", 3	},
			{"COL", 4	},
			{"TAN", 5, 1, 1, (1ul << 3)	},
			{"BTN", 6, 1, 1, (1ul << 5 | 1ul << 3)	},
			{"SIW", 7	},
			{"INS", 8	},
			{"WPS", 9	}
			/*
			{{ "COL" }, SHG_VS_COLOUR		},
			{{ "TAN" }, SHG_VS_TANGENT		},
			{{ "BTN" }, SHG_VS_BITANGENT	},
			{{ "SIW" }, SHG_VS_SKINIDXWGT	},
			{{ "INS" }, SHG_VS_INSTANCING	},
			{{ "WPS" }, SHG_VS_WORLD_POS	}
			*/
		};
	}



	static std::vector<ShaderOption> getPsOptions()
	{
		return
		{ 
			{ "LMOD",	0, 3 },
			{ "ALPHA",	3	 },
			{ "NMAP",	4, 1 },
			{ "FOG",	5	 },
			{ "SHADOW", 6	 },
			{ "GAMMA",	7	 }
			/*
			{{ "" }, SHG_PS_LIGHTMODEL },
			{{ "" }, SHG_PS_ALPHA },
			{{ "" }, SHG_PS_NORMALMAP},
			{{ "" }, SHG_PS_FOG},
			{{ "" }, SHG_PS_SHADOW},
			{{ "" }, SHG_PS_GAMMA}
			*/
		};
	}


	static bool preprocessAllPermutations(
		const std::wstring& ogFilePathW, 
		const std::string& outDirPath
		/*,std::vector<ShaderOption>& optionSet*/)	
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

		for (uint64_t i = 0; i < ( 1 << bitCount); ++i)
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



	static void CreatePermFromKey(
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
				D3D_SHADER_MACRO d3dshm{ so.name.c_str(), valStrings.back().c_str()};
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



	static void createShPerm(
		const std::string& outDirPath,
		ID3DBlob* textBuffer, 
		const std::vector<D3D_SHADER_MACRO>& permOptions, 
		uint64_t total)
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
};


// Older code
/*
std::vector<ShaderOption> options
		{
			// VS settings
			{"TEX", 0	},
			{"NRM", 1	},
			{"COL", 2	},
			{"TAN", 3	},
			{"BTN", 4	},
			{"SIW", 5	},
			{"INS", 6	},
			{"WPS", 7	}
			// PS settings
			{ "LMOD",	0, 3 },
			{ "ALPHA",	3	 },
			{ "NMAP",	4, 1 },
			{ "FOG",	5	 },
			{ "SHADOW", 6	 },
			{ "GAMMA",	7	 }
		};


					uint64_t total = 0;
			std::list<std::string> valueStrings;
			debugString = std::to_string(++counter) + ": POS ";

			// Iterate through all options and add suitable macros to
			// the list of the macros passed to the shader compiler
			for (UINT j = 0; j < optionCount; ++j)
			{
				ShaderOption& so = options[j];
				uint64_t requestedOption = so._bitmask;
				uint64_t andResult = requestedOption & i;

				// If current option fits the bitmask, add it in
				if (andResult > 0)
				{
					valueStrings.push_back(std::to_string(andResult));

					D3D_SHADER_MACRO macro{ so.name.c_str(), valueStrings.back().c_str()};

					matchingPermOptions.push_back(macro);
					debugString += so.name;
					debugString += std::to_string(andResult) + " ";

					total += andResult;
				}
			}

			// I barely remember how this works but it should eliminate doubles?
			if (!existingKeys.insert(total).second)
			{
				matchingPermOptions.clear();
				debugString.clear();
				continue;
			}

			matchingPermOptions.push_back({ NULL, NULL });	// Required by d3d api

			createShPerm(outDirPath, textBuffer, matchingPermOptions, total);

			debugString += "\n";
			OutputDebugStringA(debugString.c_str());

			matchingPermOptions.clear();
			debugString.clear();
*/
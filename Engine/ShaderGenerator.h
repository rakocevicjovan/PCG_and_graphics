#pragma once
#include "ShaderCompiler.h"
#include "FileUtilities.h"
#include <algorithm>
#include <set>
#include <string>

// Constants for external code to have some default paths
static const char* VS_PROTOSHADER = "ShGen\\VS_proto.hlsl";
static const char* PS_PROTOSHADER = "ShGen\\PS_proto.hlsl";

static const char* VS_PERMUTATIONS = "ShGen\\GeneratedVS\\";
static const char* PS_PERMUTATIONS = "ShGen\\GeneratedPS\\";

static const char* NATURAL_PERMS = "ShGen\\Natty\\";



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
		};
	}



	static std::vector<ShaderOption> getPsOptions()
	{
		return
		{ 
			{"LMOD",	0, 3 },
			{"ALPHA",	3	 },
			{"NMAP",	4, 1 },
			{"FOG",		5	 },
			{"SHADOW",	6	 },
			{"GAMMA",	7	 }
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


	static bool preprocessAllPermutations(const std::wstring& ogFilePathW, const std::string& outDirPath);


	static void CreatePermFromKey(
		const std::string& outDirPath, ID3DBlob*& textBuffer,
		const std::vector<ShaderOption>& options, uint64_t key, std::set<uint64_t>& existingKeys);


	static void createShPerm(
		const std::string& outDirPath,
		ID3DBlob* textBuffer,
		const std::vector<D3D_SHADER_MACRO>& permOptions,
		uint64_t total);
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
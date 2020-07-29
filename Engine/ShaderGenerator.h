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


struct ShaderOption
{
	std::string name;
	uint16_t _offset;
	uint16_t _numBits = 1;
	uint16_t _maxVal = 1;
	uint64_t depMask = (~0ul);
};


// Hardcoded, but rarely changes and it's the easiest way to expose it

// VS options
static const ShaderOption SHG_OPT_TEX { "TEX", 0, 3, 4 };
static const ShaderOption SHG_OPT_NRM { "NRM", 3 };
static const ShaderOption SHG_OPT_COL { "COL", 4 };
static const ShaderOption SHG_OPT_TAN { "TAN", 5, 1, 1, (1ul << 3) };
static const ShaderOption SHG_OPT_BTN { "BTN", 6, 1, 1, (1ul << 5 | 1ul << 3) };
static const ShaderOption SHG_OPT_SIW { "SIW", 7 };
static const ShaderOption SHG_OPT_INS { "INS", 8 };
static const ShaderOption SHG_OPT_WPS { "WPS", 9 };

// PS options
#define PS_O 10u // Horrible but aight for now
static const ShaderOption SHG_OPT_LMOD	{ "LMOD",	PS_O + 0, 2 };
static const ShaderOption SHG_OPT_ALPHA	{ "ALPHA",	PS_O + 2 };
static const ShaderOption SHG_OPT_FOG	{ "FOG",	PS_O + 3 };
static const ShaderOption SHG_OPT_SHD	{ "SHADOW",	PS_O + 4 };
static const ShaderOption SHG_OPT_GAMMA	{ "GAMMA",	PS_O + 5 };

// PS texture options, each 1 bit (crazy number of permutations already)
#define PS_T_O PS_O + 6u
static const ShaderOption SHG_TX_DIF { "DIF", PS_T_O + 0 };
static const ShaderOption SHG_TX_NRM { "NRM", PS_T_O + 1 };
static const ShaderOption SHG_TX_SPC { "SPC", PS_T_O + 2 };
static const ShaderOption SHG_TX_SHN { "SHN", PS_T_O + 3 };
static const ShaderOption SHG_TX_OPC { "OPC", PS_T_O + 4 };
static const ShaderOption SHG_TX_DPM { "DPM", PS_T_O + 5 };
static const ShaderOption SHG_TX_AMB { "AMB", PS_T_O + 6 };
static const ShaderOption SHG_TX_MTL { "MTL", PS_T_O + 7 };
static const ShaderOption SHG_TX_RGH { "RGH", PS_T_O + 8 };
static const ShaderOption SHG_TX_OTR { "OTR", PS_T_O + 9 };


class ShaderGenerator
{
public:

	static std::vector<ShaderOption> getVsOptions()
	{
		return
		{ 
			SHG_OPT_TEX, SHG_OPT_NRM, SHG_OPT_COL, SHG_OPT_TAN,
			SHG_OPT_BTN, SHG_OPT_SIW, SHG_OPT_INS, SHG_OPT_WPS
		};
	}


	static std::vector<ShaderOption> getPsOptions()
	{
		return
		{ 
			SHG_OPT_LMOD, SHG_OPT_ALPHA, SHG_OPT_FOG, SHG_OPT_SHD, SHG_OPT_GAMMA
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
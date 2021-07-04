#pragma once
#include "ShaderCompiler.h"
#include "FileUtilities.h"
#include "VertSignature.h"
#include "TextureMetaData.h"

class Material;
class Texture;
class MaterialTexture;	// This was not on purpose I promise...
using ShaderGenKey = uint64_t;

// Constants for external code to have some default paths
inline const wchar_t* VS_PROTOSHADER = L"ShGen\\VS_proto.hlsl";
inline const wchar_t* PS_PROTOSHADER = L"ShGen\\PS_proto.hlsl";
inline const wchar_t* COMPILED_FOLDER  = L"ShGen\\Compiled\\";
inline const wchar_t* PERMUTATIONS_FOLDER  = L"ShGen\\Natty\\";

inline const char* VS_PERMUTATIONS	= "ShGen\\GeneratedVS\\";
inline const char* PS_PERMUTATIONS	= "ShGen\\GeneratedPS\\";


struct ShaderOption
{
	const char* name;
	uint16_t _offset;
	uint16_t _numBits{ 1 };
	uint16_t _maxVal{ 1 };
	uint64_t depMask{ (~0ul) };
};


enum SHG_LIGHT_MODEL : uint8_t
{
	SHG_LM_NONE = 0u,
	SHG_LM_LAMBERT = 1u,
	SHG_LM_PHONG = 2u
};

static constexpr SHG_LIGHT_MODEL DEFAULT_LM = SHG_LM_LAMBERT;


// Hardcoded, but rarely changes and it's the easiest way to expose it

// VS options
constexpr ShaderOption SHG_OPT_TEX { "TEX", 0, 3, 4 };
constexpr ShaderOption SHG_OPT_NRM { "NRM", 3 };
constexpr ShaderOption SHG_OPT_COL { "COL", 4 };
constexpr ShaderOption SHG_OPT_TAN { "TAN", 5, 1, 1, (1ul << 3) };
constexpr ShaderOption SHG_OPT_BTN { "BTN", 6, 1, 1, (1ul << 5 | 1ul << 3) };
constexpr ShaderOption SHG_OPT_SIW { "SIW", 7 };
constexpr ShaderOption SHG_OPT_INS { "INS", 8 };

// PS options
constexpr UINT PS_O = 9u; // Horrible but aight for now;
constexpr ShaderOption SHG_OPT_LMOD	{ "LMOD",	PS_O + 0, 2 };
constexpr ShaderOption SHG_OPT_ALPHA	{ "ALPHA",	PS_O + 2 };
constexpr ShaderOption SHG_OPT_FOG	{ "FOG",	PS_O + 3 };
constexpr ShaderOption SHG_OPT_SHD	{ "SHADOW",	PS_O + 4 };
constexpr ShaderOption SHG_OPT_GAMMA	{ "GAMMA",	PS_O + 5 };

// 10 texture options, each 1 bit (crazy number of permutations already)
constexpr UINT PS_T_O = PS_O + 6u;
constexpr ShaderOption SHG_TX_DIF { "TEX_DIF", PS_T_O + 0 };
constexpr ShaderOption SHG_TX_NRM { "TEX_NRM", PS_T_O + 1 };
constexpr ShaderOption SHG_TX_SPC { "TEX_SPC", PS_T_O + 2 };
constexpr ShaderOption SHG_TX_SHN { "TEX_SHN", PS_T_O + 3 };
constexpr ShaderOption SHG_TX_OPC { "TEX_OPC", PS_T_O + 4 };
constexpr ShaderOption SHG_TX_DPM { "TEX_DPM", PS_T_O + 5 };
constexpr ShaderOption SHG_TX_AMB { "TEX_AMB", PS_T_O + 6 };
constexpr ShaderOption SHG_TX_MTL { "TEX_MTL", PS_T_O + 7 };
constexpr ShaderOption SHG_TX_RGH { "TEX_RGH", PS_T_O + 8 };
constexpr ShaderOption SHG_TX_RFL { "TEX_RFL", PS_T_O + 9 };
constexpr ShaderOption SHG_TX_RFR { "TEX_RFR", PS_T_O + 10 };

inline const std::map<TextureRole, const ShaderOption*> TEX_ROLE_TO_SHADER_OPTION
{
	{ DIFFUSE, &SHG_TX_DIF		},
	{ NORMAL,  &SHG_TX_NRM		},
	{ SPECULAR, &SHG_TX_SPC		},
	{ SHININESS, &SHG_TX_SHN	},
	{ OPACITY, &SHG_TX_OPC		},
	{ DPCM, &SHG_TX_DPM			},
	{ AMB_OCCLUSION, &SHG_TX_AMB		},
	{ METALLIC, &SHG_TX_MTL		},
	{ ROUGHNESS, &SHG_TX_RGH	},
	{ REFLECTION, &SHG_TX_RFL	},
	{ REFRACTION, &SHG_TX_RFR	}
};



class ShaderGenerator
{
private:
	static void AddToKey(const VertSignature& vertSig, ShaderGenKey& key, VAttribSemantic semantic, const ShaderOption& shOpt);

	static void EncodeVertexData(const VertSignature& vertSig, ShaderGenKey& key);

	static void EncodeTextureData(const std::vector<MaterialTexture>& texData, ShaderGenKey& key);

public:

	inline static const std::vector<ShaderOption> AllOptions
	{
		SHG_OPT_TEX, SHG_OPT_NRM, SHG_OPT_COL, SHG_OPT_TAN,
		SHG_OPT_BTN, SHG_OPT_SIW, SHG_OPT_INS,
		// Pixel shader options
		SHG_OPT_LMOD, SHG_OPT_ALPHA, SHG_OPT_FOG, SHG_OPT_SHD, SHG_OPT_GAMMA,
		// Texture options
		SHG_TX_DIF, SHG_TX_NRM, SHG_TX_SPC, SHG_TX_SHN, SHG_TX_OPC,
		SHG_TX_DPM, SHG_TX_AMB, SHG_TX_MTL, SHG_TX_RGH, SHG_TX_RFL, SHG_TX_RFR
	};

	static ShaderGenKey CreateShaderKey(const VertSignature& vertSig, const Material* mat, UINT lmIndex);

	static void CreatePermFromKey(const std::vector<ShaderOption>& options, ShaderGenKey key);

	static inline std::vector<D3D_SHADER_MACRO> ParseOptionsFromKey(
		const std::vector<ShaderOption>& options, 
		ShaderGenKey key,
		std::list<std::string>& values, 
		uint64_t& total);

	static void CreateShPerm(
		const std::wstring& outDirPath,
		ID3DBlob* textBuffer,
		const std::vector<D3D_SHADER_MACRO>& permOptions,
		const wchar_t* type,
		uint64_t total);

	// Neat for testing but there could (absolutely will) be too many to use this practically
	static bool PreprocessAllPermutations(
		const std::wstring& ogFilePathW, const std::wstring& outDirPath);

	// This one is used in loop generation, the other one standalone
	static void CreatePermFromKey(
		const std::wstring& outDirPath,
		ID3DBlob*& textBuffer,
		const std::vector<ShaderOption>& options,
		ShaderGenKey key,
		std::set<ShaderGenKey>& existingKeys);
};
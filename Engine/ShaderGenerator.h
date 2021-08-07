#pragma once

#include "VertSignature.h"
#include "TextureMetaData.h"
#include "ShaderOption.h"


class Material;
class Texture;
struct MaterialTexture;
using ShaderGenKey = uint64_t;


namespace ShGen
{

// Constants for external code to have some default paths
inline const wchar_t* VS_PROTOSHADER = L"ShGen\\VS_proto.hlsl";
inline const wchar_t* PS_PROTOSHADER = L"ShGen\\PS_proto.hlsl";
inline const wchar_t* COMPILED_FOLDER  = L"ShGen\\Compiled\\";
inline const wchar_t* PERMUTATIONS_FOLDER  = L"ShGen\\Natty\\";

inline const char* VS_PERMUTATIONS	= "ShGen\\GeneratedVS\\";
inline const char* PS_PERMUTATIONS	= "ShGen\\GeneratedPS\\";


enum LIGHT_MODEL : uint8_t
{
	LM_NONE = 0u,
	LM_LAMBERT = 1u,
	LM_PHONG = 2u
};


static constexpr LIGHT_MODEL DEFAULT_LM = LM_LAMBERT;


inline const std::map<TextureRole, const ShaderOption*> TEX_ROLE_TO_SHADER_OPTION
{
	{ DIFFUSE,			&TX_DIF	},
	{ NORMAL,			&TX_NRM	},
	{ SPECULAR,			&TX_SPC	},
	{ SHININESS,		&TX_SHN	},
	{ OPACITY,			&TX_OPC	},
	{ DPCM,				&TX_DPM	},
	{ AMB_OCCLUSION,	&TX_AMB	},
	{ METALLIC,			&TX_MTL	},
	{ ROUGHNESS,		&TX_RGH	},
	{ REFLECTION,		&TX_RFL	},
	{ REFRACTION,		&TX_RFR	}
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
		OPT_TEX, OPT_NRM, OPT_COL, OPT_TAN,
		OPT_BTN, OPT_SIW, OPT_INS,
		// Pixel shader options
		OPT_LMOD, OPT_ALPHA, OPT_FOG, OPT_SHD, OPT_GAMMA,
		// Texture options
		TX_DIF, TX_NRM, TX_SPC, TX_SHN, TX_OPC,
		TX_DPM, TX_AMB, TX_MTL, TX_RGH, TX_RFL, TX_RFR
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

}
#pragma once
#include "Shader.h"
#include "ShaderCache.h"
#include "ShaderGenerator.h"
#include "GuiBlocks.h"
#include "VertSignature.h"
#include "Material.h"



struct ShaderDescription
{
	uint16_t vsKey;
	uint8_t texRegisters[16];
	uint64_t texUVIndex : 48;
};



struct ShaderPack
{
	VertexShader* vs;
	PixelShader* ps;
};

static const std::map<TextureRole, const ShaderOption*> TEX_ROLE_TO_SHADER_OPTION
{
	{ DIFFUSE, &SHG_TX_DIF		},
	{ NORMAL,  &SHG_TX_NRM		},
	{ SPECULAR, &SHG_TX_SPC		},
	{ SHININESS, &SHG_TX_SHN	},
	{ OPACITY, &SHG_TX_OPC		},
	{ DISPLACEMENT, &SHG_TX_DPM	},
	{ AMBIENT, &SHG_TX_AMB		},
	{ METALLIC, &SHG_TX_MTL		},
	{ ROUGHNESS, &SHG_TX_RGH	},
	{ OTHER, nullptr			} //Can be loaded but not used in generation
};



class ShaderManager
{
public:

	inline static void addToKey(VertSignature& vertSig, uint64_t& key, 
		VAttribSemantic semantic, const ShaderOption& shOpt)
	{
		UINT semanticElementCount = vertSig.countAttribute(semantic);
		key |= (semanticElementCount << shOpt._offset);
	}


	static void encodeVertexData(VertSignature vertSig, uint64_t& key)
	{
		addToKey(vertSig, key, VAttribSemantic::TEX_COORD, SHG_OPT_TEX);
		addToKey(vertSig, key, VAttribSemantic::COL, SHG_OPT_COL);
		addToKey(vertSig, key, VAttribSemantic::NORMAL, SHG_OPT_NRM);
		addToKey(vertSig, key, VAttribSemantic::TANGENT, SHG_OPT_TAN);
		addToKey(vertSig, key, VAttribSemantic::BITANGENT, SHG_OPT_BTN);
		addToKey(vertSig, key, VAttribSemantic::B_IDX, SHG_OPT_SIW);
	}



	static void encodeTextureData(std::vector<RoleTexturePair>& texData, uint64_t& key)
	{
		for (const RoleTexturePair& rtp : texData)
		{
			auto iter = TEX_ROLE_TO_SHADER_OPTION.find(rtp._role);
			if (iter != TEX_ROLE_TO_SHADER_OPTION.end())
				key |= (1 << iter->second->_offset);
			else
				OutputDebugStringA("Matching shader option for texture type not found.");
		}
	}



	static void displayShaderPicker(VertSignature vertSig, Material* mat)
	{
		static uint64_t shaderKey{ 0 };

		enum SHG_LIGHT_MODEL : uint8_t
		{
			SHG_LM_NONE = 0u,
			SHG_LM_LAMBERT = 1u,
			SHG_LM_PHONG = 2u
		};

		struct LightModelIndex
		{
			std::string name;
			SHG_LIGHT_MODEL index;
		};

		static std::vector<LightModelIndex> lmiOptions
		{
			{ "NONE",		SHG_LM_NONE},
			{ "LAMBERT",	SHG_LM_LAMBERT},
			{ "PHONG",		SHG_LM_PHONG}
		};

		// Default lambert
		static LightModelIndex* selected = &lmiOptions[1];

		if (ImGui::Begin("Shader picker"))
		{
			if (ImGui::BeginCombo("Light model", selected->name.data()))
			{
				for (UINT i = 0; i < lmiOptions.size(); ++i)
				{
					if (ImGui::Selectable(lmiOptions[i].name.c_str()))
						selected = &lmiOptions[i];
				}
				ImGui::EndCombo();
			}
		}

		shaderKey |= (selected->index << SHG_OPT_LMOD._offset);

		encodeVertexData(vertSig, shaderKey);
		encodeTextureData(mat->_texDescription, shaderKey);

		ImGui::Text("Key: %lu", shaderKey);

		if (ImGui::Button("Create shader"))
		{
			ShaderGenerator::CreatePermFromKey(ShaderGenerator::vsAllOptions, shaderKey);
			ShaderGenerator::CreatePermFromKey(ShaderGenerator::psAllOptions, shaderKey);
		}

		ImGui::End();
	}
};
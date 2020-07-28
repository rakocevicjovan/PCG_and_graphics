#pragma once
#include "Shader.h"
#include "ShaderGenerator.h"
#include "GuiBlocks.h"
#include "VertSignature.h"
#include <unordered_map>

enum LightModel
{
	NONE,
	LAMBERT,
	PHONG,
	COOK_TORRANCE
};


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



class ShaderManager
{
	std::map<uint64_t, ShaderPack> _shaderMap;

public:


	static uint64_t createShaderKey(VertSignature vertSig, UINT lmi, bool ins)
	{
		bool rigged = vertSig.hasBones();
	}


	static void displayShaderPicker(VertSignature vertSig)
	{
		static uint64_t shaderKey{ 0 };

		struct LightModelIndex
		{
			std::string name;
			UINT index;
		};

		static std::vector<LightModelIndex> lmiOptions
		{
			{ "NONE", 0},
			{ "LAMBERT", 1},
			{ "PHONG", 2},
			{ "BLINN", 3}
		};

		LightModelIndex* selected = &lmiOptions[1];

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

		static bool instanced = false;
		ImGui::Checkbox("Instancing?", &instanced);

		if (ImGui::Button("Get shader key"))
		{
			shaderKey = createShaderKey(vertSig, selected->index, instanced);
		}

		ImGui::End();
	}
};
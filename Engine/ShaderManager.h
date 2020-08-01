#pragma once
#include "Shader.h"
#include "ShaderCache.h"
#include "ShaderGenerator.h"
#include "GuiBlocks.h"
#include "VertSignature.h"
#include "Material.h"
#include <memory>



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
private:

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


public:


	static void displayShaderPicker(VertSignature vertSig, Material* mat, ID3D11Device* device)
	{
		uint64_t shaderKey{ 0 };

		static const std::vector<LightModelIndex> lmiOptions
		{
			{ "NONE",		SHG_LM_NONE},
			{ "LAMBERT",	SHG_LM_LAMBERT},
			{ "PHONG",		SHG_LM_PHONG}
		};

		// Default lambert
		static const LightModelIndex* selected = &lmiOptions[1];

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

		shaderKey = ShaderGenerator::CreateShaderKey(selected->index, vertSig, mat);

		ImGui::Text("Key: %lu", shaderKey);

		if (ImGui::Button("Create shader"))
		{
			CreateShader(device, shaderKey, vertSig, mat);
		}

		ImGui::End();
	}



	static void CreateShader(ID3D11Device* device, uint64_t shaderKey, 
		VertSignature vertSig, Material* mat)
	{
		ShaderCompiler shc;
		shc.init(device);

		// VS
		D3D11_BUFFER_DESC WMBufferDesc = CBuffer::createDesc(sizeof(WMBuffer));
		CBufferMeta WMBufferMeta(0, WMBufferDesc.ByteWidth);
		WMBufferMeta.addFieldDescription(CBUFFER_FIELD_CONTENT::TRANSFORM, 0, sizeof(WMBuffer));

		ShaderGenerator::CreatePermFromKey(ShaderGenerator::AllOptions, shaderKey);
		auto vertInLayElements = vertSig.createVertInLayElements();

		std::string vsPath(NATURAL_PERMS + std::to_string(shaderKey) + "vs.hlsl");
		std::wstring vsPathW(vsPath.begin(), vsPath.end());

		VertexShader* vs = new VertexShader
		(shc, vsPathW, vertInLayElements, { WMBufferDesc });
		vs->describeBuffers({ WMBufferMeta });

		// PS
		auto samplerDescriptions = mat->createSamplerDesc();
		std::string psPath(NATURAL_PERMS + std::to_string(shaderKey) + "ps.hlsl");
		std::wstring psPathW(psPath.begin(), psPath.end());

		PixelShader* ps = new PixelShader
		(shc, psPathW, samplerDescriptions, std::vector<D3D11_BUFFER_DESC>{});

		shc.compileVS(vsPathW, vertInLayElements, vs->_vsPtr, vs->_layout);
		shc.compilePS(psPathW, ps->_psPtr);

		mat->setVS(vs);
		mat->setPS(ps);
	}
};
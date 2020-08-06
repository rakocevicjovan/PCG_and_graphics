#pragma once
#include "Shader.h"
#include "ShaderCache.h"
#include "ShaderGenerator.h"
#include "GuiBlocks.h"
#include "VertSignature.h"
#include "Material.h"
#include <memory>

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>



struct ShaderDescription
{
	uint16_t vsKey;
	uint8_t texRegisters[16];
	uint64_t texUVIndex : 48;
};


/*
void save(cereal::JSONInputArchive& ar, D3D11_INPUT_ELEMENT_DESC& ied)
{
	std::string semName(ied.SemanticName);
	ar(semName, ied.SemanticIndex,
		static_cast<UINT>(ied.Format), ied.InputSlot, ied.AlignedByteOffset,
		static_cast<UINT>(ied.InputSlotClass), ied.InstanceDataStepRate);
}

void load(cereal::JSONInputArchive& ar, D3D11_INPUT_ELEMENT_DESC& ied, std::string* str)
{
	ar(*str, ied.SemanticIndex,
		static_cast<UINT>(ied.Format), ied.InputSlot, ied.AlignedByteOffset,
		static_cast<UINT>(ied.InputSlotClass), ied.InstanceDataStepRate);
	ied.SemanticName = str->c_str();
}
*/

template<typename Archive> void serialize(Archive& ar, VAttrib& va)
{
	ar(static_cast<UINT>(va._semantic), static_cast<UINT>(va._type), va._size, va._numElements);
}


template<typename Archive> void serialize(Archive& ar, D3D11_BUFFER_DESC& bd)
{
	ar(bd.ByteWidth, static_cast<UINT>(bd.Usage),
		bd.BindFlags, bd.CPUAccessFlags, bd.MiscFlags, bd.StructureByteStride);
}

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

	static constexpr SHG_LIGHT_MODEL DEFAULT_LM = SHG_LM_LAMBERT;


	ID3D11Device* _pDevice;
	ShaderCache* _pShCache;

	std::map<uint64_t, ShaderPack> _existingShaders;

public:

	ShaderManager() {}

	void init(ID3D11Device* device, ShaderCache* cache)
	{
		_pDevice = device;
		_pShCache = cache;
	}



	void loadExistingKeys(const std::string& path)
	{
		auto shaderFiles = FileUtils::getFilesByExt(path, "cmp");

		for (auto& file : shaderFiles)
		{
			const std::string& filename = file.path().filename().string();

			UINT division = filename.find('.');

			uint64_t key = std::stoull(filename.substr(0, division));

			_existingShaders.insert({ key, ShaderPack{ nullptr, nullptr } });
		}
	}



	void getShaderAuto(VertSignature vertSig, Material* mat, SHG_LIGHT_MODEL lightModel = DEFAULT_LM)
	{
		// return something...
		getShaderByKey(ShaderGenerator::CreateShaderKey(vertSig, mat, lightModel));
	}



	void getShaderByKey(uint64_t shaderKey)
	{
		// Check if exists, then if loaded, load if needed, create if needed, return
		auto it = _existingShaders.find(shaderKey);
		
		if (it != _existingShaders.end())
		{
			ShaderPack& sp = it->second;
			if (sp.vs == nullptr)
			{
				loadFromKey(shaderKey);
			}
		}
		else
		{
			// Not quite, this has to be changed
			ShaderGenerator::CreatePermFromKey(ShaderGenerator::AllOptions, shaderKey);
		}
	}



	void loadFromKey(uint64_t shaderKey)
	{
		/*
		std::string shTypeStr = filename.substr(division - 3, 2);

		std::wstring wFileName(filename.begin(), filename.end());

		ID3DBlob* shaderBlob = shc.loadCompiledBlob(wFileName);
		if (FAILED(D3DWriteBlobToFile(shaderBlob, wFileName.c_str(), true)))
		{
			__debugbreak();
		}

		// Load shaders, can't work yet...
		if (shTypeStr == "vs")
		{
			VertexShader* vs = new VertexShader();
			vs->_vsPtr = shc.loadCompiledVS(shaderBlob);
			vs->_id = shaderKey;
			vs->_path = wFileName;
			vs->_type = SHADER_TYPE::VS;

			// These need to be loaded too :\

			//vs->_layout
			// Not sure if I can serialize it given it's a gpu object but
			// it can be recreated from std::vector<D3D11_INPUT_ELEMENT_DESC>
			//_device->CreateInputLayout(inLay.data(), inLay.size(), shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), &vs->_layout

			//vs->_cbuffers
			// This is recreatable from constant buffer metadata struct

			_shCache->addVertShader(filename, vs);
		}
		*/
	}



	static void CreateShader(ID3D11Device* device, uint64_t shaderKey, VertSignature vertSig, Material* mat)
	{
		ShaderGenerator::CreatePermFromKey(ShaderGenerator::AllOptions, shaderKey);
		auto vertInLayElements = vertSig.createVertInLayElements();

		// VS
		D3D11_BUFFER_DESC WMBufferDesc = CBuffer::createDesc(sizeof(WMBuffer));
		CBufferMeta WMBufferMeta(0, WMBufferDesc.ByteWidth);
		WMBufferMeta.addFieldDescription(CBUFFER_FIELD_CONTENT::TRANSFORM, 0, sizeof(WMBuffer));

		std::string vsPath(NATURAL_PERMS + std::to_string(shaderKey) + "vs.hlsl");
		std::wstring vsPathW(vsPath.begin(), vsPath.end());

		// Old way, not really good for this
		ShaderCompiler shc;
		shc.init(device);
		//VertexShader* vs = new VertexShader(shc, vsPathW, vertInLayElements, { WMBufferDesc });
		
		// New way, keep the blob and persist everything for reuse
		ID3DBlob* vsBlob = shc.compileToBlob(vsPathW, "vs_5_0");
		ID3D11VertexShader* d3dvs = shc.loadCompiledVS(vsBlob);
		std::string cmpVsPath(NATURAL_COMPS + std::to_string(shaderKey) + "vs.cmp");
		PersistVertexShader(cmpVsPath.c_str(), vsBlob, vertSig, { WMBufferDesc });

		VertexShader* vs = new VertexShader(device, vsBlob, vsPathW, vertInLayElements, { WMBufferDesc });
		vs->describeBuffers({ WMBufferMeta });
		vsBlob->Release();

		// PS
		auto samplerDescriptions = mat->createSamplerDescs();
		std::string psPath(NATURAL_PERMS + std::to_string(shaderKey) + "ps.hlsl");
		std::wstring psPathW(psPath.begin(), psPath.end());

		PixelShader* ps = new PixelShader
		(shc, psPathW, samplerDescriptions, std::vector<D3D11_BUFFER_DESC>{});

		mat->setVS(vs);
		mat->setPS(ps);
	}


	static void PersistVertexShader(
		const char* path, 
		ID3DBlob* blob, // d3d calls not const correct...
		const VertSignature& vertSig, 
		const std::vector<D3D11_BUFFER_DESC>& constantBufferDescs)
	{
		// Cereal can't serialize a pointer so persist as string
		char* begin = static_cast<char*>(blob->GetBufferPointer());
		std::string blobString(begin, blob->GetBufferSize());

		std::ofstream ofs(path);
		cereal::JSONOutputArchive ar(ofs);
		ar(blobString, vertSig._attributes, constantBufferDescs);
	}



	static void LoadVertexShader(std::string path)
	{
		std::ifstream ifs(path);
		cereal::JSONInputArchive ar(ifs);

		std::string blobString;
		ar(blobString);

		std::vector<D3D11_INPUT_ELEMENT_DESC> ieDescs;
		std::vector<D3D11_BUFFER_DESC> cbDescs;

		std::string* a;
		//load(ar)

		//ar(cbDescs);
	}




	// For UI
	struct LightModelIndex
	{
		std::string name;
		SHG_LIGHT_MODEL index;
	};



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

		shaderKey = ShaderGenerator::CreateShaderKey(vertSig, mat, selected->index);

		ImGui::Text("Key: %lu", shaderKey);

		if (ImGui::Button("Create shader"))
		{
			CreateShader(device, shaderKey, vertSig, mat);
		}

		ImGui::End();
	}
};
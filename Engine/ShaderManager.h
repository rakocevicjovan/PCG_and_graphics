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



struct VS_FileFormat
{
	std::string blobString;
	VertSignature vertSig;
	std::vector<D3D11_BUFFER_DESC> cbDescs;
};



struct PS_FileFormat
{
	std::string blobString;
	std::vector<D3D11_SAMPLER_DESC> sDescs;
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


template<typename Archive> void serialize(Archive& ar, D3D11_SAMPLER_DESC& sd)
{
	ar(sd.AddressU, sd.AddressV, sd.AddressW, sd.BorderColor, sd.ComparisonFunc, 
		sd.Filter, sd.MaxAnisotropy, sd.MaxLOD, sd.MinLOD, sd.MipLODBias);
}



struct ShaderPack
{
	VertexShader* vs = nullptr;
	PixelShader* ps = nullptr;
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



	void loadExistingKeys(const std::wstring& path)
	{
		auto shaderFiles = FileUtils::getFilesByExt(path, ".cmp");

		for (auto& file : shaderFiles)
		{
			const std::string& filename = file.path().filename().string();

			UINT division = filename.find('.');

			uint64_t key = std::stoull(filename.substr(0, division));

			_existingShaders.insert({ key, ShaderPack{ nullptr, nullptr } });
		}
	}



	ShaderPack* getShaderAuto(VertSignature vertSig, Material* mat, SHG_LIGHT_MODEL lightModel = DEFAULT_LM)
	{
		ShaderKey shaderKey = ShaderGenerator::CreateShaderKey(vertSig, mat, lightModel);

		ShaderPack* shPack = getShaderByKey(shaderKey);

		if (shPack)
			return shPack;

		ShaderPack newPack = CreateShader(_pDevice, shaderKey, vertSig, mat);

		auto inserted = _existingShaders.insert({ shaderKey, newPack });

		return &(inserted.first->second);
	}



	ShaderPack* getShaderByKey(ShaderKey shaderKey)
	{
		// Check if exists, check if loaded, create/load if needed, return
		auto it = _existingShaders.find(shaderKey);
		
		if (it != _existingShaders.end())	// Exists
		{
			ShaderPack& sp = it->second;
			if (sp.vs == nullptr)	// VS not loaded, load
				sp.vs = static_cast<VertexShader*>(loadFromKey(shaderKey, L"vs.cmp"));
			if (!sp.ps)				// PS not loaded, load
				sp.ps = static_cast<PixelShader*>(loadFromKey(shaderKey, L"ps.cmp"));

			return &it->second;
		}

		// Does not exist, create and add to existing
			return nullptr;
	}



	Shader* loadFromKey(ShaderKey shaderKey, const wchar_t* ext)
	{
		std::wstring wFileName = NATURAL_COMPS + std::to_wstring(shaderKey) + ext;

		// Load shaders, can't work yet...
		if (ext == L"vs.cmp")
		{
			return loadVertexShader(wFileName, shaderKey);
		}
		if (ext == L"ps.cmp")
		{
			return loadPixelShader(wFileName, shaderKey);
		}
		return nullptr;
	}



	static ShaderPack CreateShader(ID3D11Device* device, uint64_t shaderKey, VertSignature vertSig, Material* mat)
	{
		ShaderGenerator::CreatePermFromKey(ShaderGenerator::AllOptions, shaderKey);

		ShaderCompiler shc;
		shc.init(device);

		// VS
		std::wstring vsPathW(NATURAL_PERMS + std::to_wstring(shaderKey) + L"vs.hlsl");
		std::wstring cmpVsPath(NATURAL_COMPS + std::to_wstring(shaderKey) + L"vs.cmp");

		ID3DBlob* vsBlob = shc.compileToBlob(vsPathW, "vs_5_0");
		ID3D11VertexShader* d3dvs = shc.blobToVS(vsBlob);

		auto vertInLayElements = vertSig.createVertInLayElements();
		D3D11_BUFFER_DESC WMBufferDesc = CBuffer::createDesc(sizeof(WMBuffer));
		CBufferMeta WMBufferMeta(0, WMBufferDesc.ByteWidth);
		WMBufferMeta.addFieldDescription(CBUFFER_FIELD_CONTENT::TRANSFORM, 0, sizeof(WMBuffer));
		
		PersistVertexShader(cmpVsPath.c_str(), vsBlob, vertSig, { WMBufferDesc });

		VertexShader* vs = new VertexShader(device, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			vsPathW, vertInLayElements, { WMBufferDesc });
		vs->describeBuffers({ WMBufferMeta });
		vsBlob->Release();

		// PS
		std::wstring psPathW(NATURAL_PERMS + std::to_wstring(shaderKey) + L"ps.hlsl");
		std::wstring cmpPsPath(NATURAL_COMPS + std::to_wstring(shaderKey) + L"ps.cmp");

		ID3DBlob* psBlob = shc.compileToBlob(psPathW, "ps_5_0");
		ID3D11PixelShader* d3dps = shc.blobToPS(psBlob);

		auto samplerDescriptions = mat->createSamplerDescs();
		
		PixelShader* ps = new PixelShader(device, psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
			psPathW, samplerDescriptions, {});
		
		PersistPixelShader(cmpPsPath.c_str(), psBlob, samplerDescriptions, {});
		psBlob->Release();

		return ShaderPack{ vs, ps };
	}


	static void PersistVertexShader(
		const wchar_t* path, 
		ID3DBlob* blob, // d3d calls not const correct...
		const VertSignature& vertSig, 
		const std::vector<D3D11_BUFFER_DESC>& constantBufferDescs)
	{
		// Cereal can't serialize a pointer so persist as string, annoying really...
		std::string blobString(static_cast<char*>(blob->GetBufferPointer()), blob->GetBufferSize());

		std::ofstream ofs(path);
		cereal::JSONOutputArchive ar(ofs);
		ar(blobString, vertSig._attributes, constantBufferDescs);
	}



	VertexShader* loadVertexShader(const std::wstring& path, ShaderKey shaderKey)
	{
		// Temporary data loaded from a file to reconstruct the shader
		VS_FileFormat vsff;
		std::ifstream ifs(path);
		cereal::JSONInputArchive ar(ifs);
		ar(vsff.blobString, vsff.vertSig._attributes, vsff.cbDescs);

		auto inLay = vsff.vertSig.createVertInLayElements();

		VertexShader* vs = new VertexShader(_pDevice, vsff.blobString.data(), vsff.blobString.size(), path, inLay, vsff.cbDescs);
		vs->_id = shaderKey;

		_existingShaders.at(shaderKey).vs = vs;
		return vs;
	}



	static void PersistPixelShader(
		const wchar_t* path,
		ID3DBlob* blob,
		std::vector<D3D11_SAMPLER_DESC>& sDescs,
		const std::vector<D3D11_BUFFER_DESC>& cbDescs)
	{
		std::string blobString(static_cast<char*>(blob->GetBufferPointer()), blob->GetBufferSize());

		std::ofstream ofs(path);
		cereal::JSONOutputArchive ar(ofs);
		ar(blobString, sDescs, cbDescs);
	}



	PixelShader* loadPixelShader(const std::wstring& path, ShaderKey shaderKey)
	{
		std::string blobString;
		std::vector<D3D11_SAMPLER_DESC> sDescs;
		std::vector<D3D11_BUFFER_DESC> cbDescs;

		std::ifstream ifs(path);
		cereal::JSONInputArchive ar(ifs);
		ar(blobString, sDescs, cbDescs);

		PixelShader* ps = new PixelShader(_pDevice, blobString.data(), blobString.size(), path, sDescs, cbDescs);
		ps->_id = shaderKey;

		_existingShaders.at(shaderKey).ps = ps;
		return ps;
	}






	static void displayShaderPicker(VertSignature vertSig, Material* mat, ID3D11Device* device)
	{
		// For UI
		struct LightModelIndex
		{
			std::string name;
			SHG_LIGHT_MODEL index;
		};
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
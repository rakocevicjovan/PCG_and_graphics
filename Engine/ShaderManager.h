#pragma once

#include "Shader.h"
#include "ShaderGenerator.h"
#include "VertSignature.h"
#include "IMGUI/imgui.h"
#include "AssetID.h"
#include "TCache.h"
#include "IAssetManager.h"
#include "AssetLedger.h"
#include "AeonLoader.h"

class Material;

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


template<typename Archive>
void serialize(Archive& ar, D3D11_BUFFER_DESC& bd)
{
	ar(bd.ByteWidth, bd.Usage,
		bd.BindFlags, bd.CPUAccessFlags, bd.MiscFlags, bd.StructureByteStride);
}


template<typename Archive>
void serialize(Archive& ar, D3D11_SAMPLER_DESC& sd)
{
	ar(sd.AddressU, sd.AddressV, sd.AddressW, sd.BorderColor, sd.ComparisonFunc, 
		sd.Filter, sd.MaxAnisotropy, sd.MaxLOD, sd.MinLOD, sd.MipLODBias);
}


struct ShaderPack
{
	VertexShader* vs = nullptr;
	PixelShader* ps = nullptr;
};



class ShaderManager : public IAssetManager
{
private:

	AssetLedger* _assetLedger{};
	AeonLoader* _aeonLoader{};

	ID3D11Device* _pDevice{};

	std::map<ShaderGenKey, ShaderPack> _existingShaders;

public:

	TCache<Shader> _cache{};

	ShaderManager() = default;

	ShaderManager(AssetLedger& ledger/*, AssetManagerLocator& locator*/)
		: _assetLedger(&ledger) {}


	inline void init(ID3D11Device* device)
	{
		_pDevice = device;
	}

	ShaderPack getShaderByID(AssetID assetID)
	{

	}

	void loadExistingKeys(const std::wstring& path);

	ShaderPack* getShaderByData(VertSignature vertSig, Material* mat, SHG_LIGHT_MODEL lightModel = DEFAULT_LM);

	ShaderPack* getShaderByKey(ShaderGenKey shaderKey);

	Shader* loadFromKey(ShaderGenKey shaderKey, const wchar_t* ext);

	static ShaderPack CreateShader(ID3D11Device* device, uint64_t shaderKey, VertSignature vertSig, Material* mat);

	// These should probably be separated out
	static void PersistVertexShader(
		const wchar_t* path, 
		ID3DBlob* blob, // d3d calls not const correct...
		const VertSignature& vertSig, 
		const std::vector<D3D11_BUFFER_DESC>& constantBufferDescs)
	{
		// Cereal can't serialize a pointer so persist as string, annoying really...
		std::string blobString(static_cast<char*>(blob->GetBufferPointer()), blob->GetBufferSize());

		std::ofstream ofs(path, std::ios::binary);
		cereal::BinaryOutputArchive ar(ofs);
		ar(blobString, vertSig._attributes, constantBufferDescs);
	}



	VertexShader* loadVertexShader(const std::wstring& path, ShaderGenKey shaderKey)
	{
		// Temporary data loaded from a file to reconstruct the shader
		VS_FileFormat vsff;
		
		{
			//AssetHelpers::DeserializeFromFile<VS_FileFormat>(path.c_str());
			std::ifstream ifs(path, std::ios::binary);
			cereal::BinaryInputArchive ar(ifs);
			ar(vsff.blobString, vsff.vertSig._attributes, vsff.cbDescs);
		}

		auto inLay = vsff.vertSig.createVertInLayElements();

		VertexShader* vs = new VertexShader(_pDevice, vsff.blobString.data(), vsff.blobString.size(), path, inLay, vsff.cbDescs);
		// Bootleg solution, need to be persisting these better in the first place
		CBufferMeta WMBufferMeta(0, 64u);
		WMBufferMeta.addFieldDescription(0, sizeof(SMatrix));
		vs->_id = shaderKey;
		vs->describeBuffers({ WMBufferMeta });

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

		std::ofstream ofs(path, std::ios::binary);
		cereal::BinaryOutputArchive ar(ofs);
		ar(blobString, sDescs, cbDescs);
	}


	PixelShader* loadPixelShader(const std::wstring& path, ShaderGenKey shaderKey)
	{
		std::string blobString;
		std::vector<D3D11_SAMPLER_DESC> sDescs;
		std::vector<D3D11_BUFFER_DESC> cbDescs;

		{
			std::ifstream ifs(path, std::ios::binary);
			cereal::BinaryInputArchive ar(ifs);
			ar(blobString, sDescs, cbDescs);
		}

		PixelShader* ps = new PixelShader(_pDevice, blobString.data(), blobString.size(), path, sDescs, cbDescs);
		ps->_id = shaderKey;

		_existingShaders.at(shaderKey).ps = ps;
		return ps;
	}

	static void DisplayShaderPicker(VertSignature vertSig, Material* mat, ID3D11Device* device);
};
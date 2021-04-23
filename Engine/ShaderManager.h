#pragma once
#include "Shader.h"
#include "ShaderCache.h"
#include "ShaderGenerator.h"
#include "VertSignature.h"
#include "IMGUI/imgui.h"
#include <memory>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

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
void serialize(Archive& ar, VAttrib& va)
{
	ar(va._semantic, va._type, va._size, va._numElements);
}


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


	ID3D11Device* _pDevice{};
	ShaderCache* _pShCache{};

	std::map<uint64_t, ShaderPack> _existingShaders;

public:

	ShaderManager() {}

	inline void init(ID3D11Device* device, ShaderCache* cache)
	{
		_pDevice = device;
		_pShCache = cache;
	}



	void loadExistingKeys(const std::wstring& path);



	ShaderPack* getShaderAuto(VertSignature vertSig, Material* mat, SHG_LIGHT_MODEL lightModel = DEFAULT_LM);



	ShaderPack* getShaderByKey(ShaderKey shaderKey);



	Shader* loadFromKey(ShaderKey shaderKey, const wchar_t* ext);



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



	VertexShader* loadVertexShader(const std::wstring& path, ShaderKey shaderKey)
	{
		// Temporary data loaded from a file to reconstruct the shader
		VS_FileFormat vsff;
		
		{
			std::ifstream ifs(path, std::ios::binary);
			cereal::BinaryInputArchive ar(ifs);
			ar(vsff.blobString, vsff.vertSig._attributes, vsff.cbDescs);
		}

		auto inLay = vsff.vertSig.createVertInLayElements();

		VertexShader* vs = new VertexShader(_pDevice, vsff.blobString.data(), vsff.blobString.size(), path, inLay, vsff.cbDescs);
		// Bootleg solution, need to be persisting these better in the first place
		CBufferMeta WMBufferMeta(0, 64u);
		WMBufferMeta.addFieldDescription(CBUFFER_FIELD_CONTENT::TRANSFORM, 0, sizeof(SMatrix));
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



	PixelShader* loadPixelShader(const std::wstring& path, ShaderKey shaderKey)
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
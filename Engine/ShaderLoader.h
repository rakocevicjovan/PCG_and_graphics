#pragma once

#include "VertSignature.h"

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


static void PersistVertexShader(
	const wchar_t* path,
	ID3DBlob* blob, // Could be const, but d3d calls are not const correct...
	const VertSignature& vertSig,
	const std::vector<D3D11_BUFFER_DESC>& constantBufferDescs)
{
	// Cereal can't serialize a pointer so persist as string, annoying really...
	std::string blobString(static_cast<char*>(blob->GetBufferPointer()), blob->GetBufferSize());

	std::ofstream ofs(path, std::ios::binary);
	cereal::BinaryOutputArchive ar(ofs);
	ar(blobString, vertSig._attributes, constantBufferDescs);
}


static VertexShader LoadVertexShader(const std::wstring& path, ID3D11Device* device)
{
	// Temporary data loaded from a file to reconstruct the shader
	VS_FileFormat vsff;

	{
		std::ifstream ifs(path, std::ios::binary);
		cereal::BinaryInputArchive ar(ifs);
		ar(vsff.blobString, vsff.vertSig._attributes, vsff.cbDescs);
	}

	auto inLay = vsff.vertSig.createVertInLayElements();

	VertexShader vs(device, vsff.blobString.data(), vsff.blobString.size(), path, inLay, vsff.cbDescs);
	// Bootleg solution, need to be persisting these better in the first place
	CBufferMeta WMBufferMeta(0, 64u);
	WMBufferMeta.addFieldDescription(0, sizeof(SMatrix));
	vs.describeBuffers({ WMBufferMeta });

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


static PixelShader LoadPixelShader(const std::wstring& path, ID3D11Device* device)
{
	std::string blobString;
	std::vector<D3D11_SAMPLER_DESC> sDescs;
	std::vector<D3D11_BUFFER_DESC> cbDescs;

	{
		std::ifstream ifs(path, std::ios::binary);
		cereal::BinaryInputArchive ar(ifs);
		ar(blobString, sDescs, cbDescs);
	}

	PixelShader ps = PixelShader(device, blobString.data(), blobString.size(), path, sDescs, cbDescs);

	return ps;
}
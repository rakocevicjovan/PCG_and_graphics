#pragma once

#include "Shader.h"
#include "ShaderGenerator.h"
#include "VertSignature.h"
#include "AssetID.h"
#include "TCache.h"
#include "IAssetManager.h"
#include "AssetLedger.h"
#include "AeonLoader.h"
#include "ShaderLoader.h"

class Material;


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

	TCache<Shader> _cache{};
	using AssetHandle = TCache<Shader>::AssetHandle;

	AssetLedger* _assetLedger{};
	AeonLoader* _aeonLoader{};

	ID3D11Device* _pDevice{};

	std::map<ShaderGenKey, ShaderPack> _existingShaders;

public:

	ShaderManager() = default;

	ShaderManager(AssetLedger& ledger, ID3D11Device* device)
		: _assetLedger(&ledger), _pDevice(device) {}


	inline void init(ID3D11Device* device)
	{
		_pDevice = device;
	}


	void loadExistingKeys(const std::wstring& path);

	ShaderPack* getBestFit(VertSignature vertSig, Material* mat, ShGen::LIGHT_MODEL lightModel = ShGen::DEFAULT_LM);

	ShaderPack* getShaderByKey(ShaderGenKey shaderKey);

	Shader* loadFromKey(ShaderGenKey shaderKey, const wchar_t* ext);

	// Get these outtie here
	static ShaderPack CreateShader(ID3D11Device* device, uint64_t shaderKey, VertSignature vertSig, Material* mat);
};
#pragma once

#include "TCachedLoader.h"

#include "ShaderGenerator.h"
#include "VertSignature.h"

#include "Shader.h"
#include "ShaderLoader.h"


class Material;


struct ShaderPack
{
	VertexShader* vs = nullptr;
	PixelShader* ps = nullptr;
};


class ShaderManager
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
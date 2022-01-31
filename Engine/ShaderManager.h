#pragma once

#include "TCachedLoader.h"

#include "ShaderGenerator.h"
#include "VertSignature.h"

#include "Shader.h"
#include "ShaderLoader.h"


class Material;


struct ShaderPack
{
	std::shared_ptr<VertexShader> vs{};
	std::shared_ptr<PixelShader> ps{};
};

using AnyShader = std::variant<VertexShader, PixelShader>;

class ShaderManager final : public TCachedLoader<AnyShader, ShaderManager>
{
private:
	using base = TCachedLoader<AnyShader, ShaderManager>;

	ID3D11Device* _pDevice{};

	std::map<ShaderGenKey, ShaderPack> _existingShaders;

public:

	using base::base;

	ShaderManager(AssetLedger& ledger, AeonLoader& aeonLoader, ID3D11Device* device) 
		: base::TCachedLoader(ledger, aeonLoader), _pDevice(device) 
	{}


	inline void init(ID3D11Device* device)
	{
		_pDevice = device;
	}

	AnyShader loadImpl(const char* path, ShaderType type)
	{
		AnyShader shader;

		std::wstring widePath(path, path + strlen(path));
		
		if (type == ShaderType::VS)
		{
			shader = LoadVertexShader(widePath, _pDevice);
		}

		if (type == ShaderType::PS)
		{
			shader = LoadPixelShader(widePath, _pDevice);
		}

		return shader;

		// Should look kinda like this if I'm streamlining it
		//ShaderAsset shaderAsset = AssetHelpers::DeserializeFromFile<MaterialAsset, cereal::JSONInputArchive>(path);
		//return ShaderLoader::LoadMaterialFromAsset(std::move(materialAsset), _shaderManager, _textureManager);
	}

	// This might not belong here, anything that has to do with shader generation could be separate into another class
	void loadExistingKeys(const std::wstring& path);

	ShaderPack* getBestFit(VertSignature vertSig, Material* mat, ShGen::LIGHT_MODEL lightModel = ShGen::DEFAULT_LM);

	ShaderPack* getShaderByKey(ShaderGenKey shaderKey);

	std::shared_ptr<Shader> loadFromKey(ShaderGenKey shaderKey, const wchar_t* ext);

	static ShaderPack CreateShader(ID3D11Device* device, uint64_t shaderKey, VertSignature vertSig, Material* mat);
};
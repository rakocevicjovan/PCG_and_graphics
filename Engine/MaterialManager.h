#pragma once
#include "AssetLedger.h"
#include "Material.h"
#include "MaterialLoader.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "TCache.h"


class MaterialManager
{
private:

	AssetLedger* _assetLedger{};
	ShaderManager* _shaderManager{};
	TextureManager* _textureManager{};
	TCache<Material> _cache{};
	

public:

	MaterialManager() = default;


	MaterialManager(AssetLedger* assetLedger) : _assetLedger(assetLedger)
	{}


	Material* get(AssetID assetID)
	{
		// @TODO placeholder texture, perhaps? Also could be a handle, or consider a full blown proxy
		Material* result{ nullptr };

		// Check if loaded, otherwise load and cache it
		result = _cache.get(assetID);

		if (!result)
		{
			if (const std::string* path = _assetLedger->get(assetID); path)
			{
				MaterialAsset materialAsset = AssetHelpers::DeserializeFromFile<MaterialAsset, cereal::JSONInputArchive>(path->c_str());

				//result = _cache.store(assetID, material);
			}
		}

		assert(result && "Could not find a material with this ID.");

		return result;
	}


	Material fromAsset(MaterialAsset& materialAsset)
	{
		Material material;

		material._materialTextures.reserve(materialAsset._textures.size());

		for (auto& texRef : materialAsset._textures)
		{
			MaterialTexture materialTexture;
			materialTexture._metaData = texRef._texMetaData;
			materialTexture._tex = LoadTextureFromAsset(texRef._textureAssetID, *_assetLedger);

			material._materialTextures.emplace_back(std::move(materialTexture));
		}

		auto shaderPack = _shaderManager->getShaderByKey(1);

		material.setVS(shaderPack->vs);
		material.setPS(shaderPack->ps);

		material._opaque = materialAsset._opaque;

		return material;
	}
};
#pragma once

#include "AssetLedger.h"
#include "Material.h"
#include "MaterialLoader.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "TCache.h"
#include "IAssetManager.h"
#include "AssetManagerLocator.h"
#include "AeonLoader.h"


class MaterialManager : public IAssetManager
{
private:

	AssetLedger* _assetLedger{};
	AeonLoader* _aeonLoader{};

	AssetManagerLocator* _assetManagerLocator{};

public:

	TCache<Material> _cache{};

	MaterialManager() = default;

	MaterialManager(AssetLedger& ledger, AssetManagerLocator& locator, AeonLoader& aeonLoader)
		: _assetLedger(&ledger), _assetManagerLocator(&locator), _aeonLoader(&aeonLoader) {}


	MaterialManager(AssetLedger* assetLedger) : _assetLedger(assetLedger)
	{}


	Material* get(AssetID assetID)
	{
		Material* result{ nullptr };

		result = _cache.get(assetID);

		if (!result)
		{
			if (const std::string* path = _assetLedger->getPath(assetID); path)
			{
				MaterialAsset materialAsset = AssetHelpers::DeserializeFromFile<MaterialAsset, cereal::JSONInputArchive>(path->c_str());

				result = _cache.store(assetID, *MaterialLoader::LoadMaterialFromAsset(materialAsset, *_assetLedger));
			}
			else
			{
				assert(result && "Could not find a material with this ID in the asset ledger.");
			}
		}

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

			_assetManagerLocator->get<TextureManager>(AssetType::TEXTURE)->get(texRef._textureAssetID);
			material._materialTextures.emplace_back(std::move(materialTexture));
		}

		auto shaderPack = _assetManagerLocator->get<ShaderManager>(AssetType::SHADER)->getShaderByKey(1);

		material.setVS(shaderPack->vs);
		material.setPS(shaderPack->ps);

		material._opaque = materialAsset._opaque;

		return material;
	}
};
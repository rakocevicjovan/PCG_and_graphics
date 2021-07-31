#pragma once

#include "AssetLedger.h"
#include "Material.h"
#include "MaterialLoader.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "TCache.h"

//#include "IAssetManager.h"
//#include "AssetManagerLocator.h"
#include "AeonLoader.h"


class MaterialManager : public IAssetManager
{
private:

	TCache<Material> _cache{};
	using AssetHandle = TCache<Material>::AssetHandle;

	AssetLedger* _assetLedger{};
	AeonLoader* _aeonLoader{};

	//AssetManagerLocator* _assetManagerLocator{};		// , _assetManagerLocator(&locator)
	ShaderManager* _shaderManager;
	TextureManager* _textureManager;

public:

	MaterialManager() = default;

	MaterialManager(AssetLedger& ledger, ShaderManager& shaderManager, TextureManager& textureManager, AeonLoader& aeonLoader)
		: _assetLedger(&ledger), _shaderManager(&shaderManager), _textureManager(&textureManager), _aeonLoader(&aeonLoader) {}


	std::future<AssetHandle> get_async(AssetID matID)
	{
		return _aeonLoader->pushTask(
			[this](AssetID assetID)
			{
				return get(assetID);
			}, matID);
	}


	std::shared_ptr<Material> get(AssetID assetID)
	{
		std::shared_ptr<Material> result{ nullptr };

		result = _cache.get(assetID);

		if (!result)
		{
			if (const std::string* path = _assetLedger->getPath(assetID); path)
			{
				MaterialAsset materialAsset = AssetHelpers::DeserializeFromFile<MaterialAsset, cereal::JSONInputArchive>(path->c_str());
				auto material = MaterialLoader::LoadMaterialFromAsset(materialAsset, _shaderManager, _textureManager);
				result = _cache.store(assetID, material);
			}
			else
			{
				assert(result && "Could not find a material with this ID in the asset ledger.");
			}
		}

		return result;
	}


	//Material fromAsset(MaterialAsset& materialAsset)
	//{
	//	Material material;

	//	material._materialTextures.reserve(materialAsset._textures.size());

	//	for (auto& texRef : materialAsset._textures)
	//	{
	//		MaterialTexture materialTexture;
	//		materialTexture._metaData = texRef._texMetaData;
	//		materialTexture._tex = LoadTextureFromAsset(texRef._textureAssetID, *_assetLedger);

	//		//_assetManagerLocator->get<TextureManager>(AssetType::TEXTURE)->get(texRef._textureAssetID);
	//		material._materialTextures.emplace_back(std::move(materialTexture));
	//	}

	//	//auto shaderPack = _assetManagerLocator->get<ShaderManager>(AssetType::SHADER)->getShaderByKey(1);

	//	//material.setVS(shaderPack->vs);
	//	//material.setPS(shaderPack->ps);

	//	material._opaque = materialAsset._opaque;

	//	return material;
	//}
};
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

	ShaderManager* _shaderManager;
	TextureManager* _textureManager;

	std::unordered_map<AssetID, std::shared_future<AssetHandle>> _futures;

public:

	MaterialManager() = default;

	MaterialManager(AssetLedger& ledger, ShaderManager& shaderManager, TextureManager& textureManager, AeonLoader& aeonLoader)
		: _assetLedger(&ledger), _shaderManager(&shaderManager), _textureManager(&textureManager), _aeonLoader(&aeonLoader) {}


	std::shared_future<AssetHandle> get_async(AssetID assetID)
	{
		auto future = _aeonLoader->pushTask(
			[this](AssetID assetID)
			{
				return get(assetID);
			}, assetID);

		auto shared_future = future.share();

		_futures.emplace(assetID, shared_future);
		return shared_future;
	}


	AssetHandle get(AssetID assetID)
	{
		AssetHandle result{};

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
};
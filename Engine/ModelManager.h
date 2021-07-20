#pragma once
#include "ModelLoader.h"
#include "TextureLoader.h"
#include "MaterialManager.h"
#include "AeonLoader.h"
#include "Deserialize.h"
#include "IAssetManager.h"
#include "AeonLoader.h"

class ModelManager : public IAssetManager
{
private:

	AssetLedger* _assetLedger{};
	AeonLoader* _aeonLoader{};
	TCache<SkModel> _cache{};

	AssetManagerLocator* _assetManagerLocator{};
	MaterialManager* _materialManager{};

	std::map<AssetID, std::future<SkModel>> _skModelFutures;

public:

	ModelManager() = default;

	ModelManager(AssetLedger& ledger, AssetManagerLocator& locator, AeonLoader& aeonLoader, MaterialManager& matMan)
		: _assetLedger(&ledger), _assetManagerLocator(&locator), _aeonLoader(&aeonLoader), _materialManager(&matMan) {}


	SkModel* get(AssetID assetID)
	{
		SkModel* result{ nullptr };

		// Check if loaded, otherwise load and cache it
		result = _cache.get(assetID);

		if (!result)
		{
			auto* AMD = _assetLedger->get(assetID);
			auto* filePath = &AMD->path;

			if (!AMD)
			{
				assert(false && "Asset not found for given assetID.");
				return {};
			}

			auto allDeps = _assetLedger->getAllDependencies(assetID);

			// Send out an asynchronous request
			auto futureThing = _aeonLoader->request(filePath->c_str(),
				[this](const char* path) -> SkModel
				{
					auto skModelAsset = AssetHelpers::DeserializeFromFile<SkModelAsset>(path);
					auto skModel = ModelLoader::LoadSkModelFromAsset(std::move(skModelAsset), *_assetLedger);
					return skModel;
				});

			_skModelFutures.insert({ assetID, std::move(futureThing) });

			//result = _cache.store(assetID, 
		}

		return result;
	}
};
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

	//AssetManagerLocator* _assetManagerLocator{};	// , _assetManagerLocator(&locator)
	MaterialManager* _materialManager{};

	std::map<AssetID, std::future<SkModel>> _skModelFutures;

public:

	TCache<SkModel> _cache{};

	ModelManager() = default;

	ModelManager(AssetLedger& ledger, AeonLoader& aeonLoader, MaterialManager& matMan)
		: _assetLedger(&ledger), _aeonLoader(&aeonLoader), _materialManager(&matMan) {}


	std::shared_ptr<SkModel> get(AssetID assetID)
	{
		std::shared_ptr<SkModel> result{ nullptr };

		// Check if loaded, otherwise load and cache it
		result = _cache.get(assetID);

		if (!result)
		{
			auto* AMD = _assetLedger->get(assetID);

			if (!AMD)
			{
				assert(false && "Asset not found for given assetID.");
				return {};
			}

			auto& [path, deps, type] = *AMD;

			auto skModelAsset = AssetHelpers::DeserializeFromFile<SkModelAsset>(path.c_str());
			auto skModel = ModelLoader::LoadSkModelFromAsset(std::move(skModelAsset), _materialManager);

			result = _cache.store(assetID, skModel);
		}

		return result;
	}
};

// // Send out an asynchronous request
//auto futureAsset = _aeonLoader->request(filePath->c_str(),
//	[&matMan = std::ref(_materialManager)](const char* path)
//{
//	auto skModelAsset = AssetHelpers::DeserializeFromFile<SkModelAsset>(path);
//	auto skModel = ModelLoader::LoadSkModelFromAsset(std::move(skModelAsset), matMan);
//	return skModel;
//});
//
//_skModelFutures.insert({ assetID, std::move(futureAsset) });
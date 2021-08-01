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

	TCache<SkModel> _cache{};
	using AssetHandle = TCache<SkModel>::AssetHandle;

	AssetLedger* _assetLedger{};
	AeonLoader* _aeonLoader{};

	//AssetManagerLocator* _assetManagerLocator{};	// , _assetManagerLocator(&locator)
	MaterialManager* _materialManager{};

	std::unordered_map<AssetID, std::shared_future<AssetHandle>> _futures;

	std::shared_future<AssetHandle> checkIfLoading(AssetID assetID)
	{
		auto iter = _futures.find(assetID);
		if (iter != _futures.end())
		{
			return iter->second;
		}
		return {};
	}


public:


	ModelManager() = default;


	ModelManager(AssetLedger& ledger, AeonLoader& aeonLoader, MaterialManager& matMan)
		: _assetLedger(&ledger), _aeonLoader(&aeonLoader), _materialManager(&matMan) {}


	std::shared_future<AssetHandle> get_async(AssetID assetID)
	{
		auto shared_future = _aeonLoader->pushTask(
			[this](AssetID assetID)
			{
				return get(assetID);
			}, assetID).share();

		static std::mutex _futureMutex;
		_futureMutex.lock();
		_futures.emplace(assetID, shared_future);
		_futureMutex.unlock();

		return shared_future;
	}


	// Blocking function from the viewpoint of the caller. It can internally spawn threads for dependencies.
	std::shared_ptr<SkModel> get(AssetID assetID)
	{
		AssetHandle result{ nullptr };

		// Check if already loaded
		result = _cache.get(assetID);

		if (!result)
		{
			// Check if it's currently being loaded
			auto future = checkIfLoading(assetID);
			if (future.valid())
			{
				future.wait();
				return future.get();
			}

			// Not loaded or loading, issue a request
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
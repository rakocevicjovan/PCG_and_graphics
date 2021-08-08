#pragma once
#include "TCache.h"
#include "AeonLoader.h"
#include "AssetLedger.h"
#include "ModelLoader.h"
#include "MaterialManager.h"
#include "SkeletonManager.h"
#include "AnimationManager.h"
#include "Deserialize.h"
//#include "IAssetManager.h"


class ModelManager // final : public IAssetManager
{
private:

	TCache<SkModel> _cache{};
	using AssetHandle = TCache<SkModel>::AssetHandle;

	AssetLedger* _assetLedger{};
	AeonLoader* _aeonLoader{};

	std::unordered_map<AssetID, std::shared_future<AssetHandle>> _futures;

	inline static std::mutex FUTURE_MUTEX{};
	inline static std::mutex CACHE_MUTEX{};

	MaterialManager* _materialManager{};
	SkeletonManager* _skMan{};
	AnimationManager* _aniMan{};


	std::shared_future<AssetHandle> load(AssetID assetID)
	{
		auto shared_future = _aeonLoader->pushTask(
			[this](AssetID assetID)
			{
				if (const std::string* path = _assetLedger->getPath(assetID); path)
				{
					auto skModelAsset = AssetHelpers::DeserializeFromFile<SkModelAsset>(path->c_str());
					auto skModel = ModelLoader::LoadSkModelFromAsset(std::move(skModelAsset), _materialManager, _skMan, _aniMan);
					return addToCache(assetID, std::make_shared<SkModel>(std::move(skModel)));
				}
				assert(false && "Could not find an asset with this ID.");
			},
			assetID).share();

			FUTURE_MUTEX.lock();
			_futures.insert({ assetID, shared_future });
			FUTURE_MUTEX.unlock();

			return shared_future;
	}


	std::shared_future<AssetHandle> pendingOrLoad(AssetID assetID)
	{
		{
			std::lock_guard guard(FUTURE_MUTEX);
			if (auto it = _futures.find(assetID); it != _futures.end())
			{
				return it->second;
			}
		}
		return load(assetID);
	}


	inline AssetHandle getFromCache(AssetID assetID)
	{
		std::lock_guard cacheGuard(CACHE_MUTEX);
		return _cache.get(assetID);
	}


	inline AssetHandle addToCache(AssetID assetID, AssetHandle& handle)
	{
		std::lock_guard cacheGuard(CACHE_MUTEX);
		return _cache.store(assetID, handle);
	}

public:


	ModelManager() = default;


	ModelManager(AssetLedger& ledger, AeonLoader& aeonLoader, MaterialManager& matMan, SkeletonManager& skMan, AnimationManager& aniMan)
		: _assetLedger(&ledger), _aeonLoader(&aeonLoader), _materialManager(&matMan), _skMan(&skMan), _aniMan(&aniMan) {}


	std::shared_future<AssetHandle> getAsync(AssetID assetID)
	{
		if (auto existing = getFromCache(assetID); existing)
		{
			std::promise<AssetHandle> promise;
			promise.set_value(existing);
			return promise.get_future();
		}

		return pendingOrLoad(assetID);
	}


	AssetHandle getBlocking(AssetID assetID)
	{
		// Can abstract it like this but there's overhead to it	//auto future = getAsync();

		if (auto existing{ getFromCache(assetID) }; existing)
		{
			return existing;
		}

		// Check if it's currently being loaded, if not load it
		auto assetFuture = pendingOrLoad(assetID);
		return assetFuture.get();
	}
};
#pragma once

#include "AssetLedger.h"
#include "AeonLoader.h"
#include "TCache.h"


template <typename AssetType, typename TLoadFn>
class TCachedLoader
{
private:

	TCache<AssetType> _cache{};
	using AssetHandle = typename TCache<AssetType>::AssetHandle;

	AssetLedger* _assetLedger{};
	AeonLoader* _aeonLoader{};

	std::unordered_map<AssetID, std::shared_future<AssetHandle>> _futures;

	std::mutex _futureMutex{};
	std::mutex _cacheMutex{};


	std::shared_future<AssetHandle> load(AssetID assetID)
	{
		auto shared_future = _aeonLoader->pushTask(
			[this](AssetID assetID)
			{
				if (const std::string* path = _assetLedger->getPath(assetID); path)
				{
					TLoadFn(path);
				}
				assert(false && "Could not find an asset with this ID.");
			},
			assetID).share();

			_futureMutex.lock();
			_futures.insert({ assetID, shared_future });
			_futureMutex.unlock();

			return shared_future;
	}


	std::shared_future<AssetHandle> pendingOrLoad(AssetID assetID)
	{
		{
			std::lock_guard guard(_futureMutex);
			if (auto it = _futures.find(assetID); it != _futures.end())
			{
				return it->second;
			}
		}
		return load(assetID);
	}


	inline AssetHandle getFromCache(AssetID assetID)
	{
		std::lock_guard cacheGuard(_cacheMutex);
		return _cache.get(assetID);
	}


	inline AssetHandle addToCache(AssetID assetID, AssetHandle handle)
	{
		std::lock_guard cacheGuard(_cacheMutex);
		return _cache.store(assetID, *handle);
	}

public:

	TCachedLoader() = default;


	TCachedLoader(AssetLedger& assetLedger, AeonLoader& aeonLoader)
		: _assetLedger(&assetLedger), _aeonLoader(&aeonLoader) {}


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

		if (AssetHandle result{ getFromCache(assetID) }; result)
		{
			return result;
		}

		// Check if it's currently being loaded, if not load it
		auto assetFuture = pendingOrLoad(assetID);
		assetFuture.wait();
		return addToCache(assetID, assetFuture.get());
	}
};
#pragma once

#include "AssetLedger.h"
#include "AeonLoader.h"
#include "TCache.h"


template <typename AssetType, typename CRTPDerived>
class TCachedLoader
{
protected:

	using AssetHandle = typename TCache<AssetType>::AssetHandle;
	TCache<AssetType> _cache{};

	AssetLedger* _assetLedger{};
	AeonLoader* _aeonLoader{};

	std::unordered_map<AssetID, std::shared_future<AssetHandle>> _futures;

	// @TODO make not static asap
	inline static std::mutex _futureMutex{};
	inline static std::mutex _cacheMutex{};

	std::shared_future<AssetHandle> load(AssetID assetID)
	{
		auto shared_future = _aeonLoader->pushTask(
			[this](AssetID assetID)
			{
				if (const std::string* path = _assetLedger->getPath(assetID); path)
				{
					CRTPDerived& derived = *static_cast<CRTPDerived*>(this);
					auto result = std::make_shared<AssetType>(derived.loadImpl(path->c_str()));
					return addToCache(assetID, std::move(result));
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


	inline AssetHandle addToCache(AssetID assetID, const AssetHandle& handle)
	{
		std::lock_guard cacheGuard(_cacheMutex);
		return _cache.store(assetID, std::move(handle));
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
		return assetFuture.get();
	}
};


template <typename AssetType, typename LoadFunctionType>
static auto make_cached_loader(AssetLedger& assetLedger, AeonLoader& aeonLoader, const LoadFunctionType& loadFunction)
{
	return TCachedLoader<AssetType, LoadFunctionType>(assetLedger, aeonLoader, loadFunction);
}
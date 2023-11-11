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

	// @TODO these shouldn't be static or thread local, but it's ok right now as I only use one instantiated instance of this template
	inline static std::mutex _futureMutex{};
	inline static std::mutex _cacheMutex{};

	template<typename... AdditionalParams>
	std::shared_future<AssetHandle> load(AssetID assetID, AdditionalParams... params)
	{
		auto shared_future = _aeonLoader->pushTask(
			[this](AssetID assetID, AdditionalParams... params)
			{
				if (const std::string* path = _assetLedger->getPath(assetID); path)
				{
					CRTPDerived& derived = *static_cast<CRTPDerived*>(this);
					auto result = std::make_shared<AssetType>(derived.loadImpl(path->c_str(), std::forward<AdditionalParams>(params)...));
					return addToCache(assetID, std::move(result));
				}
				assert(false && "Could not find an asset with this ID.");
				return AssetHandle{};
			},
			assetID,
			std::forward<AdditionalParams>(params)...).share();

			return shared_future;
	}

	template<typename... AdditionalParams>
	std::shared_future<AssetHandle> pendingOrLoad(AssetID assetID, AdditionalParams... params)
	{
		std::lock_guard guard(_futureMutex);

		if (auto it = _futures.find(assetID); it != _futures.end())
		{
			return it->second;
		}

		auto sharedFuture = load(assetID, std::forward<AdditionalParams>(params)...);
		_futures.insert({ assetID, sharedFuture });
		return sharedFuture;
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


	template<typename... AdditionalParams>
	std::shared_future<AssetHandle> getAsync(AssetID assetID, AdditionalParams... params)
	{
		if (auto existing = getFromCache(assetID); existing)
		{
			std::promise<AssetHandle> promise;
			promise.set_value(existing);
			return promise.get_future();
		}

		return pendingOrLoad(assetID, std::forward<AdditionalParams>(params)...);
	}


	template<typename... AdditionalParams>
	AssetHandle getBlocking(AssetID assetID, AdditionalParams... params)
	{
		if (AssetHandle result{ getFromCache(assetID) }; result)
		{
			return result;
		}

		// Check if it's currently being loaded, if not load it
		auto assetFuture = pendingOrLoad(assetID, std::forward<AdditionalParams>(params)...);
		assetFuture.wait();
		return assetFuture.get();
	}
};


template <typename AssetType, typename LoadFunctionType>
static auto make_cached_loader(AssetLedger& assetLedger, AeonLoader& aeonLoader, const LoadFunctionType& loadFunction)
{
	return TCachedLoader<AssetType, LoadFunctionType>(assetLedger, aeonLoader, loadFunction);
}
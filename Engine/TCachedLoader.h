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

	// Assumption is this will never overflow :)
	std::atomic<uint32_t> _externalAssetIDCounter{};

	std::mutex _futureMutex{};
	std::mutex _cacheMutex{};

	// TODO rename
	template<typename... AdditionalParams>
	std::shared_future<AssetHandle> load(AssetID assetID, const char* assetPath, AdditionalParams... params)
	{
		auto shared_future = _aeonLoader->pushTask(
			[this](AssetID assetID, const char* assetPath, AdditionalParams... params)
			{
				CRTPDerived& derived = *static_cast<CRTPDerived*>(this);
				auto result = std::make_shared<AssetType>(derived.loadImpl(assetPath, std::forward<AdditionalParams>(params)...));
				return addToCache(assetID, std::move(result));

				//assert(false && "Could not find an asset with this ID.");
				//return AssetHandle{};
			},
			assetID,
			assetPath,
			std::forward<AdditionalParams>(params)...).share();

		return shared_future;
	}

	template<typename... AdditionalParams>
	std::shared_future<AssetHandle> pendingOrLoad(AssetID assetID, const char* path, AdditionalParams... params)
	{
		std::lock_guard guard(_futureMutex);

		if (auto it = _futures.find(assetID); it != _futures.end())
		{
			return it->second;
		}

		auto sharedFuture = load(assetID, path, std::forward<AdditionalParams>(params)...);
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

	// Using AssetID, works only for internal resources. A resource is considered internal when it's been imported and recorded in the asset ledger.
	template<typename... AdditionalParams>
	std::shared_future<AssetHandle> getAsync(AssetID assetID, AdditionalParams... params)
	{
		if (auto existing = getFromCache(assetID); existing)
		{
			std::promise<AssetHandle> promise;
			promise.set_value(existing);
			return promise.get_future();
		}

		const std::string* path = _assetLedger->getPath(assetID);
		if (!path)
		{
			assert(false && "Could not find an asset with this ID.");
			return {};
		}

		return pendingOrLoad(assetID, path->c_str(), std::forward<AdditionalParams>(params)...);
	}

	template<typename... AdditionalParams>
	AssetHandle getBlocking(AssetID assetID, AdditionalParams... params)
	{
		if (AssetHandle result{ getFromCache(assetID) }; result)
		{
			return result;
		}

		const std::string* path = _assetLedger->getPath(assetID);
		if (!path)
		{
			assert(false && "Could not find an asset with this ID.");
			return {};
		}

		// Check if it's currently being loaded, if not load it
		auto assetFuture = pendingOrLoad(assetID, path->c_str(), std::forward<AdditionalParams>(params)...);
		assetFuture.wait();
		return assetFuture.get();
	}

	// Using a path, works for both internal and external resources. Using AssetID for internal resources is recommended.
	// Internal resources loaded using this function are treated as external, resulting AssetID will be different
	template<typename... AdditionalParams>
	std::shared_future<AssetHandle> getAsync(const char* path, AdditionalParams... params)
	{
		const auto externalAssetIDHash = this->_externalAssetIDCounter++;
		const auto externalAssetID = CreateAssetID(externalAssetIDHash, AssetTypeToEnum<AssetType>(), true);

		if (auto existing = getFromCache(externalAssetID); existing)
		{
			std::promise<AssetHandle> promise;
			promise.set_value(existing);
			return promise.get_future();
		}

		return pendingOrLoad(externalAssetID, path, std::forward<AdditionalParams>(params)...);
	}

	template<typename... AdditionalParams>
	AssetHandle getBlocking(const char* path, AdditionalParams... params)
	{
		const auto externalAssetIDHash = this->_externalAssetIDCounter++;
		const auto externalAssetID = CreateAssetID(externalAssetIDHash, AssetTypeToEnum<AssetType>(), true);

		if (AssetHandle result{ getFromCache(externalAssetID) }; result)
		{
			return result;
		}

		// Check if it's currently being loaded, if not load it
		auto assetFuture = pendingOrLoad(externalAssetID, path, std::forward<AdditionalParams>(params)...);
		assetFuture.wait();
		return assetFuture.get();
	}
};


template <typename AssetType, typename LoadFunctionType>
static auto make_cached_loader(AssetLedger& assetLedger, AeonLoader& aeonLoader, const LoadFunctionType& loadFunction)
{
	return TCachedLoader<AssetType, LoadFunctionType>(assetLedger, aeonLoader, loadFunction);
}
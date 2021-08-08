#pragma once

#include "AssetLedger.h"
#include "Texture.h"
#include "TCache.h"
#include "IAssetManager.h"
#include "AeonLoader.h"


class TextureManager : public IAssetManager
{
private:
	
	TCache<Texture> _cache{};
	using AssetHandle = TCache<Texture>::AssetHandle;

	AssetLedger* _assetLedger{};
	AeonLoader* _aeonLoader{};

	std::unordered_map<AssetID, std::shared_future<AssetHandle>> _futures;
	inline static std::mutex FUTURE_MUTEX{};	// Make non static, stopgap for now cba changing construction
	inline static std::mutex CACHE_MUTEX{};

	ID3D11Device* _device{ nullptr };


	std::shared_future<AssetHandle> load(AssetID assetID)
	{
		auto shared_future = _aeonLoader->pushTask(
			[this](AssetID assetID)
			{
				if (const std::string* path = _assetLedger->getPath(assetID); path)
				{
					return addToCache(assetID, std::move(std::make_shared<Texture>(_device, *path)));
				}
				assert(false && "Could not find a texture with this ID.");
			},
			assetID).share();

		return shared_future;
	}


	std::shared_future<AssetHandle> pendingOrLoad(AssetID assetID)
	{
		std::lock_guard guard(FUTURE_MUTEX);
		
		if (auto it = _futures.find(assetID); it != _futures.end())
		{
			return it->second;
		}

		auto sharedFuture = load(assetID);
		_futures.insert({ assetID, sharedFuture });
		return sharedFuture;
	}


	inline AssetHandle getFromCache(AssetID assetID)
	{
		std::lock_guard cacheGuard(CACHE_MUTEX);
		return _cache.get(assetID);
	}


	inline AssetHandle addToCache(AssetID assetID, const AssetHandle& handle)
	{
		std::lock_guard cacheGuard(CACHE_MUTEX);
		return _cache.store(assetID, std::move(handle));
	}

public:

	TextureManager() = default;


	TextureManager(AssetLedger& assetLedger, AeonLoader& aeonLoader, ID3D11Device* device) 
		: _assetLedger(&assetLedger), _aeonLoader(&aeonLoader), _device(device) {}


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
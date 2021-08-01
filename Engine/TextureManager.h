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
	using TextureHandle = TCache<Texture>::AssetHandle;

	AssetLedger* _assetLedger{ nullptr };
	AeonLoader* _aeonLoader{};

	std::unordered_map<AssetID, std::shared_future<TextureHandle>> _futures;

	// Remove both of these and make them not static, this is a stopgap! @TODO
	inline static std::mutex FUTURE_MUTEX{};
	inline static std::mutex CACHE_MUTEX{};

	ID3D11Device* _device{ nullptr };


	std::shared_future<TextureHandle> load(AssetID assetID)
	{
		auto shared_future = _aeonLoader->pushTask(
			[this](AssetID assetID)
			{
				if (const std::string* path = _assetLedger->getPath(assetID); path)
				{
					return addToCache(assetID, std::make_shared<Texture>(_device, *path));
				}
				assert(false && "Could not find a texture with this ID.");
			},
			assetID).share();

		FUTURE_MUTEX.lock();
		_futures.insert({ assetID, shared_future });
		FUTURE_MUTEX.unlock();

		return shared_future;
	}


	std::shared_future<TextureHandle> pendingOrLoad(AssetID assetID)
	{
		{
			std::lock_guard futuresGuard(FUTURE_MUTEX);

			if (auto it = _futures.find(assetID); it != _futures.end())
			{
				return it->second;
			}
		}

		return load(assetID);
	}


	inline TextureHandle getFromCache(AssetID assetID)
	{
		std::lock_guard cacheGuard(CACHE_MUTEX);
		return _cache.get(assetID);
	}


	inline TextureHandle addToCache(AssetID assetID, TextureHandle handle)
	{
		std::lock_guard cacheGuard(CACHE_MUTEX);
		return _cache.store(assetID, *handle);
	}

public:

	TextureManager() = default;


	TextureManager(AssetLedger& assetLedger, AeonLoader& aeonLoader, ID3D11Device* device) 
		: _assetLedger(&assetLedger), _aeonLoader(&aeonLoader), _device(device) {}


	std::shared_future<TextureHandle> getAsync(AssetID assetID)
	{
		std::shared_future<TextureHandle> result;
		
		if (auto existing = getFromCache(assetID); existing)
		{
			std::promise<TextureHandle> promise;
			promise.set_value(existing);
			result = promise.get_future();
			return result;
		}

		// They will get put in the cache by an update()? It isn't written yet @TODO - think about this design!
		return pendingOrLoad(assetID);
	}


	TextureHandle getBlocking(AssetID assetID)
	{
		// Can abstract it like this but there's overhead to it	//auto future = getAsync();
		
		TextureHandle result{ getFromCache(assetID) };

		if (!result)
		{
			// Check if it's currently being loaded, if not load it
			auto assetFuture = pendingOrLoad(assetID);
			assetFuture.wait();
			result = addToCache(assetID, assetFuture.get());
		}

		return result;
	}
};
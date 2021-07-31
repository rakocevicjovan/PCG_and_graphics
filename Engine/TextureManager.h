#pragma once

#include "AssetLedger.h"
#include "Texture.h"
#include "TCache.h"
#include "IAssetManager.h"
#include "AeonLoader.h"


class TextureManager : public IAssetManager
{
private:
	using AssetHandle = TCache<Texture>::AssetHandle;
	TCache<Texture> _cache{};

	AssetLedger* _assetLedger{ nullptr };
	AeonLoader* _aeonLoader{};

	ID3D11Device* _device{ nullptr };

public:

	TextureManager() = default;

	/*, AssetManagerLocator& locator*/
	TextureManager(AssetLedger& assetLedger, AeonLoader& aeonLoader, ID3D11Device* device) 
		: _assetLedger(&assetLedger), _aeonLoader(&aeonLoader), _device(device) {}


	std::future<AssetHandle> get_async(AssetID textureID)
	{
		return _aeonLoader->pushTask(
			[this](AssetID assetID)
			{
				return get(assetID);
			}, textureID);
	}


	AssetHandle get(AssetID textureID)
	{
		// @TODO placeholder texture, perhaps? Also could be a handle, or consider a full blown proxy
		AssetHandle result{ nullptr };

		// Check if loaded, otherwise load and cache it
		result = _cache.get(textureID);

		if(!result)
		{
			if (const std::string* path = _assetLedger->getPath(textureID); path)
			{
				result = _cache.store(textureID, Texture(_device, *path));
			}
		}

		assert(result && "Could not find a texture with this ID.");

		return result;
	}
};
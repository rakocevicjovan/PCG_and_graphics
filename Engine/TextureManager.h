#pragma once
#include "AssetLedger.h"
#include "Texture.h"
#include "TCache.h"


class TextureManager
{
private:

	AssetLedger* _assetLedger{ nullptr };
	ID3D11Device* _device{ nullptr };
	TCache<Texture> _cache{};

public:

	TextureManager() = default;


	TextureManager(AssetLedger* assetLedger, ID3D11Device* device) : _assetLedger(assetLedger), _device(device)
	{}


	Texture* get(AssetID textureID)
	{
		// @TODO placeholder texture, perhaps? Also could be a handle, or consider a full blown proxy
		Texture* result{ nullptr };	

		// Check if loaded, otherwise load and cache it
		result = _cache.get(textureID);

		if(!result)
		{
			if (const std::string* path = _assetLedger->get(textureID); path)
			{
				result = _cache.store(textureID, Texture(_device, *path));
			}
		}

		assert(result && "Could not find the texture with this ID");

		return result;
	}
};
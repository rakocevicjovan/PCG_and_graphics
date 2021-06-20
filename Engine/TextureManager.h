#pragma once
#include <unordered_map>
#include "AssetLedger.h"
#include "Texture.h"


class TextureManager
{
public:

	TextureManager() = default;


	TextureManager(AssetLedger* assetLedger, ID3D11Device* device)
		: _assetLedger(assetLedger), _device(device)
	{}


	Texture* get(AssetID textureID)
	{
		Texture* result{ nullptr };	// @TODO placeholder texture, perhaps? Also could be a handle, or consider a full blown proxy

		auto iter = _texMap.find(textureID);

		// Check if loaded, otherwise yeet to the ledger
		if (iter != _texMap.end())
		{
			result = &iter->second;
		}
		else
		{
			const std::string* path = _assetLedger->get(textureID);

			if (path)
			{
				result = &(_texMap.insert({ textureID, Texture(_device, *path) }).first->second);
			}
		}

		return result;
	}


	bool create(const std::string& path, const Texture& texture)
	{
		AssetID nameHash = _assetLedger->insert(path.c_str(), ResType::TEXTURE);
		return _texMap.insert({ nameHash, texture }).second;
	}

private:

	std::unordered_map<AssetID, Texture> _texMap;
	AssetLedger* _assetLedger{ nullptr };
	ID3D11Device* _device{nullptr};
};
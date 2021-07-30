#pragma once

#include "IAssetManager.h"
#include "AssetType.h"


class AssetManagerLocator
{
private:

	std::unordered_map<AssetType, IAssetManager*> _assetManagerMap;

public:

	bool registerManagerForType(AssetType type, IAssetManager* assetManager)
	{
		auto it = _assetManagerMap.find(type);

		if (it == _assetManagerMap.end())
		{
			_assetManagerMap.insert({ type, assetManager });
			return true;
		}
		else
		{
			if (assetManager == it->second)
			{
				return false;	// Already registered
			}
			else
			{
				assert(false && "Attempting to overwrite asset manager for asset. This is not allowed. Because I said so.");
			}
		}
	}

	template <typename T>
	T* get(AssetType assetType)
	{
		auto it = _assetManagerMap.find(assetType);

		if (it == _assetManagerMap.end())
		{
			assert(false && "Asset manager for requested asset type was not found.");
			return nullptr;
		}

		return static_cast<T*>(it->second);
	}

};
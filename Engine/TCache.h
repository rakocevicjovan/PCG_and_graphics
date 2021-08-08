#pragma once


template <typename AssetType>
class TCache
{
public:

	using AssetHandle = std::shared_ptr<AssetType>;

private:

	std::unordered_map<AssetID, AssetHandle> _cache;
	//std::mutex _cMut;	// Consider if I should lock internally or externally

public:

	bool exists(AssetID assetID)
	{
		return (_cache.find(assetID) != _cache.end());
	}


	AssetHandle store(AssetID assetID, const AssetHandle& assetType)
	{
		auto [iter, success] = _cache.try_emplace(assetID, std::move(assetType));
		assert(success && "An asset with this ID already exists in the cache.");
		return iter->second;
	}


	AssetHandle store_or_get(AssetID assetID, AssetType& assetType)
	{
		auto [iter, success] = _cache.try_emplace(assetID, std::make_shared<AssetType>(std::move(assetType)));
		return iter->second;
	}


	AssetHandle get(AssetID assetID)
	{
		auto iter = _cache.find(assetID);
		if (iter != _cache.end())
		{
			return iter->second;
		}
		return nullptr;
	}


	bool erase(AssetID assetID)
	{
		auto iter = _cache.find(assetID);
		if (iter != _cache.end())
		{
			_cache.erase(iter);
		}
	}


	void overwrite(AssetID assetID, AssetType& type)
	{
		auto iter = _cache.find(assetID);

		assert(iter != _cache.end() && "Asset your are trying to overwrite was not found");

		iter->second = type;
	}
};
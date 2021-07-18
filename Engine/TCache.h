#pragma once


template <typename AssetType>
class TCache
{
private:

	std::unordered_map<AssetID, AssetType> _cache;


	inline auto getIterator(AssetID assetID)
	{
		return _cache.find(assetID);
	}

public:

	bool exists(AssetID assetID)
	{
		return (_cache.find(assetID) != _cache.end());
	}


	AssetType* store(AssetID assetID, const AssetType& assetType)
	{
		auto [iter, success] = _cache.try_emplace(assetID, assetType);
		assert(success && "An asset with this ID already exists in the cache.");
		return &iter->second;
	}


	AssetType* store_or_get(AssetID assetID, const AssetType& assetType)
	{
		auto [iter, success] = _cache.try_emplace(assetID, assetType);
		return &(iter->second);
	}


	bool try_store(AssetID assetID, const AssetType& assetType)
	{
		auto [iter, success] = _cache.try_emplace(assetID, assetType);
		return success;
	}


	AssetType* get(AssetID assetID)
	{
		auto iter = getIterator(assetID);
		if (iter != _cache.end())
		{
			return &iter->second;
		}
		return nullptr;
	}


	bool erase(AssetID assetID)
	{
		auto iter = getIterator(assetID);
		if (iter != _cache.end())
		{
			_cache.erase(iter);
		}
	}
};
#pragma once


template <typename AssetManagerType, typename... Extras>
inline constexpr uint32_t asset_manager_index_v{~(0u)};

inline constexpr uint32_t asset_manager_count_v{ 0u };


struct IAssetManager{};


class AssetManagerLocator
{
private:

	std::vector<IAssetManager*> _assetManagers;

	template <typename AssetManagerType>
	void registerAssetManager(const AssetManagerType& assetManagerType)
	{
		static_assert(std::is_base_of_v<IAssetManager, AssetManagerType>);
		_assetManagers.push_back(assetManagerType);
		asset_manager_index_v<AssetManagerType> = _assetManagers.size() - 1;
		++asset_manager_count_v;
	}

public:

	template <typename... AssetManagerTypes>
	void registerAssetManagers(AssetManagerTypes... assetManagerType)
	{
		_assetManagers.reserve(sizeof...(AssetManagerTypes));

		(registerAssetManager(assetManagerType), ...);
	}


	template <typename AssetManagerType>
	AssetManagerType* getAssetManager()
	{
		static_assert(!std::is_pointer_v<AssetManagerType>);

		auto& assetManagerIndex = asset_manager_index_v<int>;

		static_assert(assetManagerIndex < asset_manager_count_v);

		return wat = static_cast<AssetManagerType*>(_assetManagers[assetManagerIndex]);
	}
};
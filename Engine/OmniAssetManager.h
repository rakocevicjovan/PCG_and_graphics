#pragma once

#include "AssetLedger.h"
#include "AeonLoader.h"
#include "AssetManagerLocator.h"
#include "AssetType.h"


// One to rule them all and in darkness bind them. Connect all loaders and managers here, with the ledger and possibly using the locator.
//template <typename... Types>
class OmniAssetManager
{
public:

	AeonLoader* _loader;
	AssetLedger* _assetLedger;
	
	AssetManagerLocator* _locator;

public:

	OmniAssetManager(AeonLoader& aeonLoader, AssetLedger& assetLedger, AssetManagerLocator& locator) : 
		_loader(&aeonLoader),
		_assetLedger(&assetLedger),
		_locator(&locator)
	{}

	template <typename AssetType>
	std::shared_ptr<AssetType> request(AssetID assetID)
	{
		// Get asset metadata first
		auto* assetMetaData = _assetLedger->get(assetID);
		auto& [path, deps, assetType] = *assetMetaData;

		// Get the asset manager for this type of asset
		auto* manager = _locator->get(assetType);

		assert(manager && "Manager for given asset type not found.");

		std::future<std::shared_ptr<AssetType>> future = _loader->request(path.c_str(),
		[&manager, assetID](const char* path)
		{
			auto asset = manager->get(assetID);
			return std::reinterpret_pointer_cast<AssetType>(asset);
		});
		
		future.wait();
		return future.get();
	}
};
#pragma once

#include "AssetLedger.h"
#include "AeonLoader.h"
#include "AssetManagerLocator.h"
#include "AssetType.h"

#include <tuple>



// One to rule them all and in darkness bind them. Connect all loaders and managers here, with the ledger and possibly using the locator.
//template <typename... Types>
class OmniAssetManager
{
private:

	AeonLoader* _loader;
	AssetLedger* _assetLedger;
	
	//AssetManagerLocator* _locator;
	//std::tuple<Types...> _managers;


public:

	OmniAssetManager(AeonLoader& aeonLoader, AssetLedger& assetLedger, AssetManagerLocator& locator) : /*Types&&... managers*/
		_loader(&aeonLoader),
		_assetLedger(&assetLedger)//,
		//_locator(&locator)
		//_managers(std::make_tuple(std::move(managers)...))
	{
		/*static_assert(((std::is_base_of_v<IAssetManager, Types>) & ...));*/
	}



	AssetType* request(AssetID assetID)
	{
		// Check if loaded, otherwise load and cache it
		//auto* manager = _locator.get<bonk>();
		//auto& cache = manager->_cache;

		//AssetType* result = cache.get(assetID);

		//if (!result)
		//{
		//	auto* AMD = _assetLedger->get(assetID);
		//	auto* filePath = &AMD->path;

		//	if (!AMD)
		//	{
		//		assert(false && "Asset not found for given assetID.");
		//		return {};
		//	}

		//	auto dependencies = _assetLedger->getAllDependencies(assetID);

		//	for (auto& dep : dependencies)
		//	{
		//		auto* curManager = _locator->getForType<dep->type>();
		//		auto futureAsset = getAsset(dep);
		//	}

			// This should be ok as well
			//auto allDeps = _assetLedger->getAllDependencies(assetID);
		//}

		return nullptr;
	}
};
#pragma once
#include "Skeleton.h"
#include "AssetLedger.h"
#include "Deserialize.h"


// Skeleton is persisted as-is for now and has no specific asset
static std::unique_ptr<Skeleton> LoadTextureFromAsset(AssetID id, const AssetLedger& assetLedger)
{
	auto path = assetLedger.get(id);

	auto skeleton = std::make_unique<Skeleton>();
	*skeleton = DeserializeFromFile<Skeleton>(path->c_str());
	
	return std::move(skeleton);
}
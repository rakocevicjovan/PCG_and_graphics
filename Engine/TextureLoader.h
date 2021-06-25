#pragma once
#include "Texture.h"
#include "AssetLedger.h"

// Just a stand in, will be expanded on and texture cleaned up (needs it a lot)
static std::shared_ptr<Texture> LoadTextureFromAsset(AssetID id, const AssetLedger& assetLedger)
{
	std::shared_ptr<Texture> result = std::make_shared<Texture>();

	auto path = assetLedger.get(id);

	result->loadFromFile(nullptr, path->c_str());

	return result;
}
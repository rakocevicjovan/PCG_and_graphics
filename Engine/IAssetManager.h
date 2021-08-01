#pragma once
#include "AssetID.h"

class IAssetManager
{
private:

public:
	// This makes all assets inherit from a base class. It's not really nice, especially when returning a shared_ptr, requires ugly hacks.
	//virtual std::shared_ptr<void> get(AssetID assetID) = 0;
};
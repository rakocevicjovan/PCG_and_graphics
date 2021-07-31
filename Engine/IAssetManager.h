#pragma once
#include "AssetID.h"

class IAssetManager
{
private:

public:
	// This makes all assets inherit from a base class. It's not really nice, especially when returning a shared_ptr.
	//virtual std::shared_ptr<void> get(AssetID assetID) = 0;
	
	std::shared_ptr<void> get(AssetID assetID)
	{
		return nullptr;
	}
};
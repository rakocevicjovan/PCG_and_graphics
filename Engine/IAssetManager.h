#pragma once
#include "AssetID.h"

class IAssetManager
{
private:

public:

	// @todo delete this, just a stopgap
	void* get(AssetID assetID)
	{
		return nullptr;
	}
};
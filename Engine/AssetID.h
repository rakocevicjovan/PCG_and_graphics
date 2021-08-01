#pragma once
#include "AssetType.h"


using AssetID = uint64_t;
static constexpr AssetID NULL_ASSET = (~static_cast <AssetID>(0)); // Can collide though...

// This is not used now. See AssetLedger.h for AssetMetaData struct
struct ResourceDef
{
	AssetID _id;
	std::string _path;
	EAssetType _type;

	template<typename Archive>
	void serialize(Archive& ar)
	{
		ar(_id, _path, _type);
	}
};
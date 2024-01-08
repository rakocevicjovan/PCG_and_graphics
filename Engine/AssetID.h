#pragma once
#include "AssetType.h"


using AssetID = uint64_t;
static constexpr AssetID NULL_ASSET = (~static_cast <AssetID>(0)); // Can collide though...

// Untracked is true if the asset is an external asset which can't be found in the ledger via usual imported formats.
// Such assets are usually loaded via paths to transfer/conversion formats such as png, jpg etc... 
// Their AssetID does not correspond to a persisted AssetID in the ledger. They will be assigned one instead.
inline AssetID CreateAssetID(uint32_t assetHash32, EAssetType type, bool untracked = false)
{
	static_assert(sizeof(AssetID) == 8);
	static_assert(sizeof(EAssetType) == 1);
	
	AssetID assetID{ static_cast<AssetID>(untracked) };
	assetID |= static_cast<AssetID>(assetHash32) << 32;
	assetID |= static_cast<AssetID>(type) << 24;
	
	return assetID;
}

// This is not used in the main path now. See AssetLedger.h for AssetMetaData struct
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
#pragma once

#include "Blob.h"
#include "ViewStreamBuffer.h"

namespace AssetHelpers
{

	// AssetType is intended to be any "AssetSomething" type but technically anything with implemented serialize function works
	// ArchiveType is any type of cereal archive, but will mostly be either cereal::BinaryInputArchive or cereal::JSONInputArchive
	template <typename AssetType, typename ArchiveType = cereal::BinaryInputArchive>
	static AssetType DeserializeFromFile(const char* path)
	{
		std::ifstream ifs(path, std::ios::binary);
		ArchiveType inputArchive(ifs);

		AssetType asset;
		asset.serialize(inputArchive);	// Don't get confused. This call actually deserializes when archive type is input!

		return asset;
	}

	template <typename AssetType, typename ArchiveType = cereal::BinaryInputArchive>
	static AssetType DeserializeFromBlob(Blob&& blob)
	{
		ViewStreamBuffer vsb(assetLedgerBlob._data.get(), assetLedgerBlob.size());
		std::istream istream(&vsb);
		cereal::ArchiveType ia(istream);

		AssetType asset;
		asset.serialize(ia);

		return asset;
	}
}
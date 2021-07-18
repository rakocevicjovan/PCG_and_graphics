#pragma once

#include "Blob.h"

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
		assert(false); // This won't work! Need to write my own I think... because pubsetbuf does nothing. But it would be useful to have
		std::istringstream iss(std::ios_base::binary | std::ios_base::beg);
		
		char* wat = blob.dataAsType<char>();
		iss.rdbuf()->pubsetbuf(blob.dataAsType<char>(), blob.size());

		cereal::BinaryInputArchive bia(iss);

		AssetType asset;
		asset.serialize(bia);

		return asset;
	}
}
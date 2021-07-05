#pragma once
#include "AssetID.h"
#include "FileUtilities.h"

// Not sure if in progress is required, might be good to differentiate between some
enum class LoadingStatus : uint8_t
{
	QUEUED = 0u,
	IN_PROGRESS = 1u,
	FINISHED = 2u
};


struct LoadingJob
{
	LoadingStatus loadingStatus;
	Blob blob;
};

// Parallelized (to be) file reader, nothing more for now
class AeonLoader
{
	std::unordered_map<AssetID, LoadingJob> _loadingJobs;

public:

	void requestAsset(AssetID assetID);
	LoadingStatus queryLoadingStatus(AssetID assetID);
	Blob claimLoadedAsset(AssetID assetID);
};
#include "pch.h"

#include "AeonLoader.h"


// Not currently in use, disregard.
void AeonLoader::requestAsset(AssetID assetID, const char* path)
{}


void AeonLoader::update()
{
	
}


//LoadingStatus AeonLoader::queryLoadingStatus(AssetID assetID)
//{
//	return _loadingJobs.at(assetID).loadingStatus;
//}


//Blob AeonLoader::claimLoadedAsset(AssetID assetID)
//{
//	auto iter = _loadingJobs.find(assetID);
//
//	if (iter != _loadingJobs.end())
//	{
//		LoadingJob& loadingJob = iter->second;
//
//		if (loadingJob.loadingStatus == LoadingStatus::FINISHED)
//		{
//			Blob result = std::move(loadingJob.blob);
//
//			_loadingJobs.erase(iter);
//
//			return result;
//		}
//	}
//
//	return Blob{};
//}
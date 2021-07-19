#include "pch.h"

#include "AeonLoader.h"


// Not currently in use, disregard.
void AeonLoader::requestAsset(AssetID assetID, const char* path)
{
	auto futureBlob = _threadPool.push(std::bind(
		[&](const char* path)
		{
			return FileUtils::readAllBytes(path);
		}, 
		path));

	_jobQueMutex.lock();
	_futures.insert({assetID, std::move(futureBlob)});
	_jobQueMutex.unlock();
}


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
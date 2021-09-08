#pragma once

#include "AssetID.h"
#include "VitThreadPool.h"
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

// Parallelized (to be) file reader, nothing more for now... might replace LoadingJob with a lambda
class AeonLoader
{
	ctpl::thread_pool _threadPool;

	std::mutex _jobQueMutex;
	std::unordered_map<AssetID, std::future<Blob>> _futures;
	//std::unordered_map<AssetID, LoadingJob> _loadingJobs;

public:

	AeonLoader(uint32_t maxThreadCount = 4u) : _threadPool(maxThreadCount) {}

	void resizeThreadPool(uint32_t maxThreadCount)
	{
		_threadPool.resize(std::min(maxThreadCount, std::thread::hardware_concurrency()));
	}

	
	template <typename Task, typename... TaskArgs>
	auto pushTask(Task&& task, TaskArgs... args)
	{
#ifdef DEBUG
		((_RPT1(0, "AssetID when pushing task: %" PRIu64 "!\n", std::forward<TaskArgs>(args))), ...);
#endif
		auto result = _threadPool.push(
			std::bind(
				[](int threadNum, const Task& task, AssetID assetID)
				{
#ifdef DEBUG
					_RPT1(0, "In thread %d: AssetID is : %" PRIu64 "!\n", threadNum, assetID);
#endif
					return task(assetID);
				}, 
				std::placeholders::_1,
				task,
				std::forward<TaskArgs>(args)...)
		);

		return std::move(result);
	}
};
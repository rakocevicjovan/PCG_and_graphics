#pragma once

#include "AssetID.h"
#include "VitThreadPool.h"
#include "FileUtilities.h"

// Not sure if in progress is required, might be good to have
enum class LoadingStatus : uint8_t
{
	QUEUED = 0u,
	IN_PROGRESS = 1u,
	FINISHED = 2u
};


class AeonLoader
{
	ctpl::thread_pool _threadPool;

	std::mutex _jobQueMutex;
	std::unordered_map<AssetID, std::future<Blob>> _futures;

public:

	AeonLoader(uint32_t maxThreadCount = 4u) : _threadPool(maxThreadCount) {}

	void resizeThreadPool(uint32_t maxThreadCount)
	{
		_threadPool.resize(std::min(maxThreadCount, std::thread::hardware_concurrency()));
	}

	
	template <typename Task, typename... TaskArgs>
	auto pushTask(Task&& task, AssetID assetId, TaskArgs... args)
	{
#ifdef _DEBUG
		((_RPT1(0, "AssetID when pushing task: %" PRIu64 "!\n", assetId)));
#endif
		auto result = _threadPool.push(
			std::bind(
				[](int threadNum, const Task& task, AssetID assetId, TaskArgs... args)
				{
#ifdef _DEBUG
					_RPT1(0, "In thread %d: AssetID is : %" PRIu64 "!\n", threadNum, assetId);
#endif
					return task(assetId, std::forward<TaskArgs>(args)...);
				}, 
				std::placeholders::_1,
				task,
				assetId,
				std::forward<TaskArgs>(args)...)
		);

		return result;
	}
};
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

	template <typename Fn, typename... Args>
	constexpr auto makeTaskForFn(const Fn& fn, Args... args)
	{
		return
			[&](int threadNum)
		{
			return fn(args...);
		};
	}

public:

	AeonLoader(uint32_t maxThreadCount = 4u) : _threadPool(maxThreadCount) {}

	void resizeThreadPool(uint32_t maxThreadCount)
	{
		_threadPool.resize(std::min(maxThreadCount, std::thread::hardware_concurrency()));
	}


	template <typename OnLoadedCallback>
	auto request(const char* path, const OnLoadedCallback& callback)
	{
		return
			std::move(
				_threadPool.push(
					std::bind(
						[&callback](const char* path)
						{
							return callback(path);
						},
						path)
				)
			);
	}

	
	template <typename Task, typename... TaskArgs>
	auto pushTask(const Task& task, TaskArgs... args)
	{
		auto threadTask = makeTaskForFn(task, std::forward<TaskArgs>(args)...);
		auto result = _threadPool.push(threadTask);
		return result;
	}
};
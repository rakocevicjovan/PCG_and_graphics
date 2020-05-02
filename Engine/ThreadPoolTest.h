#pragma once
#include "ThreadPool.h"
#include "VitThreadPool.h"
#include <future>
#include <debugapi.h>

#include "ParallelPrefixSum.h"
#include "ClusterManager.h"
#include "Math.h"

class ThreadPoolTest
{

	/*
	// This is not how it works but I didn't update it yet! Incorrect use of function.
	static void multiTester(UINT totalSize)
	{
		// Obviously this will use a pool when testing is done, not spawn every frame!
		auto nThreads = std::thread::hardware_concurrency();

		UINT chunk = totalSize / nThreads;

		std::vector<std::thread> _threads;
		std::vector<std::promise<UINT>> _promises;
		std::vector<std::future<UINT>> _futures;

		_threads.resize(nThreads);
		_promises.resize(nThreads);
		_futures.reserve(nThreads);

		for (int i = 0; i < nThreads; i++)
		{
			UINT start = i * chunk;
			UINT end = start + chunk;	//min((i + 1u) * chunk, _gridSize - 1);

			_futures.push_back(_promises[i].get_future());

			// For member functions
			//_threads[i] = new std::thread(&PrefixSumParallel::prefixSumParallel, this, start, end, std::move(_promises[i]));

			// For free functions
			//_threads[i] = std::thread(&prefixSumParallel, start, end, std::move(_promises[i]));

			_threads.emplace_back(&prefixSumParallel, i * chunk, (i + 1u) * chunk, _promises[i]);

		}

		for (int i = 0; i < nThreads; i++)
		{
			_threads[i].join();
			UINT wat = _futures[i].get();

			char buffer[50];
			sprintf(buffer, "Result is: %d \n", wat);

			OutputDebugStringA(buffer);
		}
	}
	*/



public:

	ctpl::thread_pool _tp;
	static const UINT _nThreads = 4u;
	std::vector<std::future<void>> _results;

	ThreadPoolTest() : _tp(_nThreads)
	{
		_results.resize(_nThreads);
	}
};
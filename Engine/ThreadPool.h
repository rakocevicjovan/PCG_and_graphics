#pragma once
#include <thread>
#include <mutex>
#include <vector>
#include <deque>
#include <condition_variable>

typedef unsigned int UINT;


template <typename ...JobFuncArgs> class ThreadPool
{
private:

	UINT _numThreads;
	
	std::vector<std::thread> _pool;
	std::deque<std::function<void(), JobFuncArgs>> _queue;			// std::function<void()>
	
	std::mutex _queMutex;
	std::mutex _poolMutex;

	std::condition_variable _condition;

	bool _stop = false;


public:

	ThreadPool(uint8_t numThreads = 0u)
	{
		// Default value is the number of hardware threads 
		_numThreads = numThreads == 0 ? (std::thread::hardware_concurrency() - 1u) : (numThreads);

		for (int i = 0; i < _numThreads; ++i)
			_pool.push_back(std::thread(&ThreadPool::looper));
	}



	ThreadPool(const ThreadPool&) = delete;



	ThreadPool& operator=(const ThreadPool&) = delete;



	~ThreadPool()
	{
		if (!_pool.empty())
			shutdown();
	}



	void looper()
	{
		while (true)
		{
			std::unique_lock<std::mutex> lock(_queMutex);

			// Wait for the condition (there's a job scheduled or the thread pool is terminating)
			_condition.wait(lock, 
				[&]	{
						return (!_queue.empty() || _stop);
					}
			);

			auto job = _queue.front();
			_queue.pop_front();

			lock.unlock();

			job(); // function<void()> type
		}
	}


	// Pool_Obj.Add_Job(std::bind(&Some_Class::Some_Method, &Some_object));
	void addJob(std::function<void()> job)
	{
		std::unique_lock<std::mutex> lock(_queMutex);
		_queue.push_back(job);
		lock.unlock();
		_condition.notify_one();
	}



	void shutdown()
	{
		std::unique_lock<std::mutex> lock(_poolMutex);
		_stop = true;	// use this flag in condition.wait to terminate threads
		lock.unlock();

		_condition.notify_all(); // wake up all threads.

		// Join all threads.
		for (std::thread &thread : _pool)
			thread.join();

		_pool.clear();
	}



};
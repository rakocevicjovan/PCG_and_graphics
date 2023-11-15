#pragma once
#include <condition_variable>

template<typename Derived>
class Pausable
{
private:
	std::mutex _conditionMutex;
	std::condition_variable _conditionVariable;
	bool _running{ true };
	bool _done{ false };

public:

	void pause()
	{
		{
			std::lock_guard lock(_conditionMutex);
			_running = false;
		}
		_conditionVariable.notify_one();
	}

	void resume()
	{
		{
			std::lock_guard lock(_conditionMutex);
			_running = true;
		}
		_conditionVariable.notify_one();
	}

	template <typename... Params>
	void run(Params... params)
	{
		while (!_done)
		{
			std::unique_lock unique_lock(_conditionMutex);

			//constexpr auto max_wait_time = std::chrono::duration(std::chrono::microseconds{1'000'000});
			//auto is_not_paused  = _conditionVariable.wait_for(unique_lock, max_wait_time,
			//	[&]()
			//	{
			//		return _running;	// Returns false if paused, blocking the thread
			//	});
			//if (is_not_paused){}else{}

			_conditionVariable.wait(unique_lock,
				[&]()
				{
					//OutputDebugStringA("Paused\n");
					//std::cout << "Paused\n";
					return _running;
				});
			unique_lock.unlock();

			auto& derived = *static_cast<Derived*>(this);
			derived.doWork(std::forward<Params>(params)...);

			_conditionVariable.notify_one();
		}
	}

	// While this is safe to call since it's read only, it won't necessarily be true by the time we check the result unless called only by the thread which writes the bool.
	bool is_running() const
	{
		return _running;
	}
};
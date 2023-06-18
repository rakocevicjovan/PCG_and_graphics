#pragma once
#include <cstdint>
#include <vector>

class FPSCounter
{
private:
	uint16_t _frameCount{ 0u };
	uint16_t _numTrackedFrames{32u};
	std::vector<float> _frameTimes;
	float _avgFrameTime{ 0.f };
	float _denom{1.f / 32.f};

public:

	FPSCounter(uint16_t nFramesToTrack = 32u)
	{
		setNumTrackedFrames(nFramesToTrack);
	}

	// Averages n frames in constant time, more frames tracked use more memory but don't take longer
	void tickFast(float frameTime)
	{
		_frameCount = (++_frameCount) % _numTrackedFrames;

		float oldVal = _frameTimes[_frameCount];
		float newVal = frameTime;
		_frameTimes[_frameCount] = newVal;

		_avgFrameTime += (newVal - oldVal) * _denom;
	}

	// Doesn't accumulate errors over time
	void tickAccurate(float frameTime)
	{
		_frameCount = (++_frameCount) % _numTrackedFrames;
		_frameTimes[_frameCount] = frameTime;

		float accumulated{ 0.f };

		for (auto i = 0; i < _numTrackedFrames; ++i)
		{
			accumulated += _frameTimes[i];
		}

		_avgFrameTime = accumulated * _denom;
	}


	void setNumTrackedFrames(uint16_t numTrackedFrames)
	{
		_numTrackedFrames = numTrackedFrames;
		_denom = 1.f / static_cast<float>(numTrackedFrames);

		_frameTimes.resize(numTrackedFrames);
		reset();
	}


	// Can be used occasionally to refresh the average or fix accumulated error from fast tick.
	void reset()
	{
		_frameCount = 0u;
		_avgFrameTime = 0.f;
		std::fill(_frameTimes.begin(), _frameTimes.end(), 0.f);
	}


	inline float getAverageFrameTime()
	{
		return _avgFrameTime;
	}


	inline float getAverageFPS()
	{
		return (1.f / _avgFrameTime);
	}


	uint16_t getNumTrackedFrames()
	{
		return _numTrackedFrames;
	}
};

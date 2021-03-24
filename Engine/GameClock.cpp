#include "pch.h"
#include <windows.h>
#include "GameClock.h"



GameClock::GameClock()
	: _secondsPerCount(0.0), _deltaTime(-1.0), _baseTime(0),
	_pausedAt(0), _prevTime(0), _currTime(0), _isStopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	_secondsPerCount = 1.0 / (double)countsPerSec;
}



// Returns the total time elapsed since Reset() was called, NOT counting any
// time when the clock is stopped.
float GameClock::TotalTime()const
{
	// If we are stopped, do not count the time that has passed since we stopped.
	// Moreover, if we previously already had a pause, the distance 
	// mStopTime - mBaseTime includes paused time, which we do not want to count.
	// To correct this, we can subtract the paused time from mStopTime:  
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mStopTime    mCurrTime

	if (_isStopped)
	{
		return (float)(((_stoppedAt - _pausedAt) - _baseTime)*_secondsPerCount);
	}

	// The distance mCurrTime - mBaseTime includes paused time,
	// which we do not want to count.  To correct this, we can subtract 
	// the paused time from mCurrTime:  
	//
	//  (mCurrTime - mPausedTime) - mBaseTime 
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mCurrTime

	else
	{
		return (float)(((_currTime - _pausedAt) - _baseTime)*_secondsPerCount);
	}
}



float GameClock::DeltaTime()const
{
	return (float)_deltaTime;
}



void GameClock::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	_baseTime = currTime;
	_prevTime = currTime;
	_stoppedAt = 0;
	_isStopped = false;
}



void GameClock::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);


	// Accumulate the time elapsed between stop and start pairs.
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  mBaseTime       mStopTime        startTime     

	if (_isStopped)
	{
		_pausedAt += (startTime - _stoppedAt);

		_prevTime = startTime;
		_stoppedAt = 0;
		_isStopped = false;
	}
}



void GameClock::Stop()
{
	if (!_isStopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		_stoppedAt = currTime;
		_isStopped = true;
	}
}



void GameClock::Tick()
{
	if (_isStopped)
	{
		_deltaTime = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	_currTime = currTime;

	// Time difference between this frame and the previous.
	_deltaTime = (_currTime - _prevTime) * _secondsPerCount;

	// Prepare for next frame.
	_prevTime = _currTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then mDeltaTime can be negative.
	if (_deltaTime < 0.0)
		_deltaTime = 0.0;
}
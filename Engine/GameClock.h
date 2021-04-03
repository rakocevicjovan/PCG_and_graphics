#pragma once
class GameClock
{
public:
	GameClock();

	float totalTime() const; // in seconds
	float deltaTime() const; // in seconds

	void reset(); // Call before message loop.
	void start(); // Call when unpaused.
	void stop();  // Call when paused.
	void tick();  // Call every frame.

private:
	double _secondsPerCount;
	double _deltaTime;

	__int64 _baseTime;
	__int64 _pausedAt;
	__int64 _stoppedAt;
	__int64 _prevTime;
	__int64 _currTime;

	bool _isStopped;
};


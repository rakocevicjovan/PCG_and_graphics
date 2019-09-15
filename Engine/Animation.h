#pragma once
#include "Math.h"
#include "MeshDataStructs.h"
#include <string>
#include <vector>
#include <map>


struct AnimChannel
{
	AnimChannel() {}

	AnimChannel(int p, int r, int s)
	{
		posVec.reserve(p);
		rotVec.reserve(r);
		sclVec.reserve(s);
	}

	std::string jointName;
	std::vector<std::pair<float, SVec3>> posVec;
	std::vector<std::pair<float, SQuat>> rotVec;
	std::vector<std::pair<float, SVec3>> sclVec;
};


class Animation
{
	
	std::string _name;
	double _ticks, _ticksPerSecond, _duration, _tickDuration, _invTickDuration;
	std::map<std::string, AnimChannel> _channels;
	float _elapsed;

public:

	Animation();

	Animation(std::string& name, double ticks, double ticksPerSecond, int nc) : _name(name), _ticks(ticks), _ticksPerSecond(ticksPerSecond), _elapsed(0.0f)
	{
		_duration = _ticks / _ticksPerSecond;
		_tickDuration = _duration / _ticks;
		_invTickDuration = 1.0 / _duration;
		//_channels.reserve(nc);
	}



	~Animation();


	void addChannel(const AnimChannel& ac)
	{
		_channels.insert({ ac.jointName, ac });
	}



	void update(float dTime)
	{
		_elapsed += dTime;
		_elapsed = fmod(_elapsed, _duration);
	}

	float getTickDuration() const { return _tickDuration; }
	float getElapsed()		const { return _elapsed; }

	bool getAnimChannel(std::string name, AnimChannel& ac) const 
	{ 
		auto it = _channels.find(name);
		
		if (it != _channels.end())
		{
			ac = it->second;
			return true;
		}
		return false;
	}
};
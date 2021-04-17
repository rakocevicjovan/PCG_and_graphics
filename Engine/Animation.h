#pragma once
#include "Math.h"
#include "MeshDataStructs.h"
#include "AnimChannel.h"
#include <string>
#include <vector>
#include <map>

class Bone;


class Animation
{
private:

	std::string _name;

	// @TODO not really runtime friendly, this...
	std::map<std::string, AnimChannel> _channels;

	double _ticks			{ 0. };
	double _ticksPerSecond	{ 0. };
	double _duration		{ 0. };
	double _tickDuration	{ 0. };

public:

	Animation() {};


	Animation(std::string& name, double ticks, double ticksPerSecond, int nc) : _name(name), _ticks(ticks), _ticksPerSecond(ticksPerSecond)
	{
		_duration = _ticks / _ticksPerSecond;
		_tickDuration = _duration / _ticks;
	}

	void getTransformAtTime(const Bone& bone, const SMatrix& parentMatrix, const SMatrix& glInvT, float elapsed, std::vector<SMatrix>& vec) const;

	inline void addChannel(const AnimChannel& ac) { _channels.insert({ ac._boneName, ac }); }

	inline float getNumTicks()		const { return _ticks; };
	inline float getTicksPerSec()	const { return _ticksPerSecond; }
	inline float getTickDuration()	const { return _tickDuration; }
	inline float getDuration()		const { return _duration; }
	inline uint32_t getNumChannels()	const { return _channels.size(); }
	inline std::string getName()	const { return _name; }


	const AnimChannel* getAnimChannel(const std::string& name) const
	{ 
		const AnimChannel* channel = nullptr;

		auto it = _channels.find(name);
		
		if (it != _channels.end())
			channel = &(it->second);

		return channel;
	}



	const std::map<std::string, AnimChannel>* getChannels()
	{
		return &_channels;
	}


	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(_name, _channels, _ticks, _ticksPerSecond, _duration, _tickDuration);
	}
};
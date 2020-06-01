#pragma once
#include "Math.h"
#include "MeshDataStructs.h"
#include <string>
#include <vector>
#include <map>

class Bone;

struct AnimChannel
{
	AnimChannel() {}

	AnimChannel(int p, int r, int s)
	{
		pKeys.reserve(p);
		rKeys.reserve(r);
		sKeys.reserve(s);
	}

	std::string jointName;

	std::vector<std::pair<float, SVec3>> sKeys;
	std::vector<std::pair<float, SQuat>> rKeys;
	std::vector<std::pair<float, SVec3>> pKeys;
};


class Animation
{
private:

	std::string _name;

	std::map<std::string, AnimChannel> _channels;

	double _ticks, _ticksPerSecond, _duration, _tickDuration;
	

public:

	Animation();

	Animation(std::string& name, double ticks, double ticksPerSecond, int nc) : _name(name), _ticks(ticks), _ticksPerSecond(ticksPerSecond)
	{
		_duration = _ticks / _ticksPerSecond;
		//_tickDuration = 1. / ticksPerSecond;
		_tickDuration = _duration / _ticks;
	}



	~Animation();



	void getTransformAtTime(Bone& joint, std::vector<SMatrix>& vec, const SMatrix& parentMatrix, const SMatrix& glInvT, float elapsed) const;



	SMatrix getInterpolatedTransform(const AnimChannel& channel, float currentTick, float t) const;



	inline void addChannel(const AnimChannel& ac) { _channels.insert({ ac.jointName, ac }); }



	inline float getTickDuration()	const { return _tickDuration; }



	inline float getDuration()		const { return _duration; }



	inline std::string getName() { return _name; }



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
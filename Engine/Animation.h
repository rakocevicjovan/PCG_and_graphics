#pragma once
#include "Math.h"
#include "MeshDataStructs.h"
#include <string>
#include <vector>
#include <map>

class Bone;

struct PosFrame
{
	SVec3 pos;
	float tick;

	PosFrame() {}
	PosFrame(float tick, SVec3 pos) : pos(pos), tick(tick) {}
};



struct RotFrame
{
	SQuat rot;
	float tick;

	RotFrame() {}
	RotFrame(float tick, SQuat rot) : rot(rot), tick(tick) {}
};



struct SclFrame
{
	SVec3 scale;
	float tick;

	SclFrame() {}
	SclFrame(float tick, SVec3 scale) : scale(scale), tick(tick) {}
};



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

	std::vector<SclFrame> sKeys;
	std::vector<RotFrame> rKeys;
	std::vector<PosFrame> pKeys;
};



class Animation
{
private:

	std::string _name;

	// @TODO Make this a sorted vector with lower_bound search, this is horrible!
	std::map<std::string, AnimChannel> _channels;

	double _ticks, _ticksPerSecond, _duration, _tickDuration;
	

public:

	Animation();

	Animation(std::string& name, double ticks, double ticksPerSecond, int nc) : _name(name), _ticks(ticks), _ticksPerSecond(ticksPerSecond)
	{
		_duration = _ticks / _ticksPerSecond;
		_tickDuration = _duration / _ticks;
	}



	~Animation();



	void getTransformAtTime(Bone& joint, std::vector<SMatrix>& vec, const SMatrix& parentMatrix, const SMatrix& glInvT, float elapsed) const;



	SMatrix getInterpolatedTransform(const AnimChannel& channel, float currentTick, float t) const;



	inline void addChannel(const AnimChannel& ac) { _channels.insert({ ac.jointName, ac }); }



	inline float getNumTicks()		const { return _ticks; };



	inline float getTicksPerSec()	const { return _ticksPerSecond; }



	inline float getTickDuration()	const { return _tickDuration; }



	inline float getDuration()		const { return _duration; }



	inline std::string getName()	const { return _name; }



	inline UINT getNumChannels()	const { return _channels.size(); }



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



	const std::map<std::string, AnimChannel>* getChannels()
	{
		return &_channels;
	}
};
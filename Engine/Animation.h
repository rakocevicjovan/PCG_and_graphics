#pragma once
#include "Math.h"
#include "MeshDataStructs.h"
#include <string>
#include <vector>
#include <map>

class Animation
{
	
	std::string name;
	double _ticks, _ticksPerSecond, _duration;

	std::map<std::string, Joint> jointMap;


public:

	Animation();

	Animation(std::string& name, double ticks, double ticksPerSecond) : name(name), _ticks(ticks), _ticksPerSecond(ticksPerSecond) 
	{
		_duration = _ticks / _ticksPerSecond;
	}

	~Animation();

	void init();
	void interpolate(float dTime);
};


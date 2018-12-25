#pragma once
#include "Math.h"
#include "MeshDataStructs.h"
#include <vector>
#include <map>

class Animation{

	float elapsed, duration;

public:
	Animation();
	~Animation();

	void init();
	void interpolate(float dTime);
	

	std::map<unsigned int, std::vector<Joint>> jointMap;
	std::vector<std::pair<unsigned int, std::vector<Joint>>> jointVec;
};


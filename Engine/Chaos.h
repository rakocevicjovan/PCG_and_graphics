#pragma once

#include <random>
#include <vector>

class Chaos
{

private:

	static std::random_device randomDevice;
	static std::mt19937_64 RNGesus;
	std::uniform_real_distribution<float> dist;

public:

	Chaos(float min = 0.f, float max = 1.f);

	inline float rollTheDice();
	void setRange(float min, float max);
	void fillVector(std::vector<float>& target);
};
#pragma once

#include <random>
#include <vector>

class Chaos
{
private:

	static std::random_device randomDevice;
	static std::mt19937_64 RNGesus;
	std::uniform_real_distribution<float> dist;
	std::uniform_int_distribution<int> intDist;

public:

	Chaos(float min = 0.f, float max = 1.f);
	void setRange(float min, float max);

	float roll();
	void roll_n(float* target, int count);

	int roll_int();
	void roll_int_n(int* dest, int count, int min, int max);
};
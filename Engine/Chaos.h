#pragma once

#include <random>

class Chaos
{

private:

	static std::random_device randomDevice;
	static std::mt19937_64 RNGesus;
	

public:

	Chaos() 
	{
		RNGesus = std::mt19937_64(randomDevice());
	}

	float rollTheDice(float min, float max) 
	{	
		return std::discrete_distribution<>(min, max)(RNGesus);
	}

};
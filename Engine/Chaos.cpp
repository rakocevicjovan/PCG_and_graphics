#include "Chaos.h"

std::random_device Chaos::randomDevice;
std::mt19937_64 Chaos::RNGesus(randomDevice());


Chaos::Chaos(float min, float max) 
{
	dist = std::uniform_real_distribution<float>(min, max);
}



void Chaos::setRange(float min, float max)
{
	dist = std::uniform_real_distribution<float>(min, max);
}



inline float Chaos::rollTheDice()
{	
	return dist(RNGesus);
}



void Chaos::fillVector(std::vector<float>& target) {

	for (int i = 0; i < target.size(); i++)
		target[i] = rollTheDice();
}

#include "pch.h"
#include "Chaos.h"

std::random_device Chaos::randomDevice;
std::mt19937_64 Chaos::RNGesus(randomDevice());

Chaos::Chaos(float min, float max) 
{
	dist = std::uniform_real_distribution<float>(min, max);
	intDist = std::uniform_int_distribution<int>(static_cast<int>(min), static_cast<int>(max));
}

void Chaos::setRange(float min, float max)
{
	dist = std::uniform_real_distribution<float>(min, max);
}

float Chaos::roll()
{	
	return dist(RNGesus);
}

void Chaos::roll_n(float* dest, int count)
{
	for (int i = 0; i < count; i++)
	{
		dest[i] = roll();
	}
}

int Chaos::roll_int()
{
	return intDist(RNGesus);
}

void Chaos::roll_int_n(int* dest, int count, int min, int max)
{
	for (int i = 0; i < count; i++)
	{
		dest[i] = intDist(RNGesus);
	}
}

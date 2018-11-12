#pragma once
#include "lightclass.h"

class Celestials
{
public:
	Celestials();

	PointLight Sun, Moon, Current;
	LightData Warm;

	void Interpolate(PointLight pl1, PointLight pl2, float TOD);
};


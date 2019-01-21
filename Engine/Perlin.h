#pragma once
#include "Math.h"
#include <vector>

namespace Procedural 
{

	class Perlin
	{
		//2d constants, not sure if I need this...
		const SVec2
			top = SVec2(0, 1),
			left = SVec2(-1, 0),
			bottom = SVec2(0, -1),
			right = SVec2(1, 0);
			
		std::vector<int> p;


	public:



		Perlin();
		~Perlin();

		double fade(double t);
		double perlin1d(double x);
		SVec2 perlin2d(SVec2 pos);
	};

}
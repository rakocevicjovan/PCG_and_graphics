#pragma once
#include "Math.h"
#include <vector>

namespace Procedural 
{

	class Perlin
	{
		
	private:

		static std::vector<SVec2> gradients2D; 
		static std::vector<int> hashTable;

		inline int Perlin::hash2D(int x, int y) const;

	public:

		Perlin();
		~Perlin();

		double fade(double t);
		double perlin1d(double x);
		float perlin2d(SVec2 pos);
	};

}
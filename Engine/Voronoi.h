#pragma once
#include <vector>
#include "Math.h"
#include "Mesh.h"

namespace Procedural
{

	class Voronoi
	{
	private:
		
		unsigned int _numSeeds;
		float _xSize, _ySize;
		std::vector<SVec2> _seeds;
		Mesh mesh;

	public:
		Voronoi();
		~Voronoi();

		void init(unsigned int numSeeds, float maxX, float maxY);
	};

}
#include "Voronoi.h"
#include "Chaos.h"

namespace Procedural
{

	Voronoi::Voronoi()
	{
	}


	Voronoi::~Voronoi()
	{
	}

	void Voronoi::init(unsigned int numSeeds, float maxX, float maxY)
	{
		_numSeeds = numSeeds;
		_xSize = maxX;
		_ySize = maxY;
		_seeds.resize(_numSeeds);

		std::vector<float>axes, eyes;

		Chaos c;
		c.setRange(0, _xSize);
		c.fillVector(axes);

		c.setRange(0, _ySize);
		c.fillVector(eyes);

		for (int i = 0; i < _numSeeds; ++i)
		{
			_seeds[i].x = axes[i];
			_seeds[i].y = eyes[i];
		}
	}

}
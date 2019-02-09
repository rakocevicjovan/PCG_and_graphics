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

		std::vector<float>axes(_numSeeds), eyes(_numSeeds);

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



	void Voronoi::shatter(const std::vector<SVec2>& positions)
	{
		areaIndices.resize(positions.size());

		for (int i = 0; i < positions.size(); ++i) 
		{
			float minDistSquared = pow(_seeds[0].x - positions[i].x, 2) + pow(_seeds[0].y - positions[i].y, 2);
			areaIndices[i] = 0;

			for (int j = 1; j < _seeds.size(); ++j)
			{
				float curDistSquared = pow(_seeds[j].x - positions[i].x, 2) + pow(_seeds[j].y - positions[i].y, 2);
				if (curDistSquared < minDistSquared)
					areaIndices[i] = j;
			}	
		}
	}

}
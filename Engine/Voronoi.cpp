#include "pch.h"
#include "Voronoi.h"
#include "Chaos.h"



namespace Procedural
{
	void Voronoi::init(unsigned int numSeeds, float maxX, float maxY)
	{
		_numSeeds = numSeeds;
		_xSize = maxX;
		_ySize = maxY;
		_seeds.resize(_numSeeds);

		std::vector<float> axes(_numSeeds);
		std::vector<float> eyes(_numSeeds);

		Chaos c;
		c.setRange(0, _xSize);
		c.roll_n(axes.data(), _numSeeds);

		c.setRange(0, _ySize);
		c.roll_n(eyes.data(), _numSeeds);

		for (auto i = 0u; i < _numSeeds; ++i)
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
			float minDistSquared = powf(_seeds[0].x - positions[i].x, 2) + powf(_seeds[0].y - positions[i].y, 2);
			areaIndices[i] = 0;

			for (int j = 1; j < _seeds.size(); ++j)
			{
				float curDistSquared = powf(_seeds[j].x - positions[i].x, 2) + powf(_seeds[j].y - positions[i].y, 2);
				if (curDistSquared < minDistSquared)
					areaIndices[i] = j;
			}	
		}
	}
}

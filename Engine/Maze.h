#pragma once
#include "Math.h"
#include "Model.h"
#include "Chaos.h"
#include <vector>
#include <map>


namespace Procedural
{

	struct MazeCell
	{
		MazeCell() { }
		MazeCell(int set, int x, int z) : set(set), x(x), z(z) { }
		MazeCell(int x, int z) : x(x), z(z) { }

		int set = -1;
		int x = 0, z = 0; 

		bool t = true;
		bool r = true;
	};

	struct EllerSet
	{
		std::vector<MazeCell*> cells;

		EllerSet() {}
		EllerSet(MazeCell& cell) { cells.push_back(&cell); }
		
		void VerticalMerge()
		{
			Chaos chaos;

			bool done = false;

			for (auto& c : cells)
			{
				if (chaos.rollTheDice() > .5f)
				{
					c->t = false;
					done = true;
				}
			}

			if (!done)
				cells[floor(chaos.rollTheDice() * (cells.size() - 1))]->t = false;
		}

	};



class Maze
{
	std::vector<MazeCell> cells;
	unsigned int _w, _h;
	float _cellSize;

public:

	Maze() {};
	Maze(unsigned int w, unsigned int h);
	~Maze();

	void Init(unsigned int w, unsigned int h, float cellSize);
	
	void Eller();
};

}
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
		std::vector<int> cellIDs;

		EllerSet() {}
		EllerSet(int cellID) { cellIDs.push_back(cellID); }
		
		void VerticalMerge(std::vector<MazeCell>& cells)
		{
			Chaos chaos;

			bool done = false;

			for (auto& CID : cellIDs)
			{
				if (chaos.rollTheDice() > .5f)
				{
					cells[CID].t = false;
					done = true;
				}
			}

			//no cells in the set were opened -> randomly pick one from the range [0, size-1] and open it to connect the set
			if (!done)
				cells[floor(chaos.rollTheDice() * (cellIDs.size() - 1))].t = false;
		}

	};



class Maze
{
	std::vector<MazeCell> cells;
	unsigned int _w, _h;
	float _cellSize;

	Model model;

public:

	Maze() {};
	Maze(unsigned int w, unsigned int h);
	~Maze();

	void Init(unsigned int w, unsigned int h, float cellSize);
	
	void Eller();
	void PopulateRow(int z, std::map<int, EllerSet>& currentRow);
	void CreateModel();
};

}
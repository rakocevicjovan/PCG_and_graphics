#include "Maze.h"
#include <numeric>

namespace Procedural
{

	Maze::Maze(unsigned int w, unsigned int h) :_w(w), _h(h)
	{
		Eller();
	}



	Maze::~Maze()
	{

	}



	void Maze::Init(unsigned int w, unsigned int h, float cellSize)
	{ 
		_w = w;
		_h = h;
		_cellSize = cellSize; 
	}



	void Maze::Eller()
	{
		Chaos c;

		cells.resize(_w * _h);
		std::map<int, EllerSet> sets;
		
		//assign a set to each of the cells in the first row uniquely
		for (int x = 0; x < _w; ++x)
		{
			cells[x] = MazeCell(x, 0);
			cells.back().set = x;
			sets.insert(std::map<int, EllerSet>::value_type(x, EllerSet(cells.back())));
		}

		//iterate through all rows and keep bridging vertically and horizontally
		for (int z = 0; z < _h - 1; ++z)
		{
			int offset = z * _w;

			for (int x = 0; x < _w - 1; ++x)
			{
				int index = offset + x;
				
				if (cells[index].set < 0)
					cells[index].set = index;

				sets.insert(std::map<int, EllerSet>::value_type(cells[index].set, EllerSet(cells[index])));
			}


			for (int x = 0; x < _w - 1; ++x)
			{
				int index = offset + x;
				MazeCell currentCell = cells[index];

				int currentSetID = currentCell.set;
				int nextSetID = cells[index + 1].set;

				//50% chance to join two different sets (can't join self in order to prevent looping)
				if (currentSetID != nextSetID)
				{
					if (c.rollTheDice() > 0.5f)
					{
						//notify the cells about their new set
						for (auto& cell : sets.at(nextSetID).cells) cell->set = currentSetID;

						//move the cells from their old set to their new set (aka merge next into current)
						sets.at(currentSetID).cells.insert(sets.at(currentSetID).cells.end(), sets.at(nextSetID).cells.begin(), sets.at(nextSetID).cells.end());

						//next set is now obsolete, as it has been merged, so it is erased from the map
						sets.erase(nextSetID);

						//right wall is removed from the current cell as a consequence of merging
						cells[index].r = false;
					}
				}
			}

			//iterate existing sets with their indices
			for (auto& es : sets)
			{
				//merge each of them vertically
				es.second.VerticalMerge();

				//search for merged cells
				for (auto setCell : es.second.cells)
				{
					if (!setCell->t)	//if a cell is merged, add the one above to the set, but also set the set in it, so the data stays consistent
					{
						cells[(setCell->z + 1) * _w + setCell->x].set = es.first;
						es.second.cells.push_back(&cells[(setCell->z + 1) * _w + setCell->x]);
					}
						
				}
			}


			//cells.insert(cells.end(), currentRow.begin(), currentRow.end());
			//currentRow = nextRow;
		}

	}

}
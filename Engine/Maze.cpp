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
		cells.resize(_w * _h);

		Chaos c;
		std::map<int, EllerSet> currentRow;

		for (int z = 0; z < _h - 1; ++z)
		{
			PopulateRow(z, currentRow);

			for (int x = 0; x < _w - 1; ++x)
			{
				int index = z * _w + x;
				int curSet = cells[index].set, nxtSet = cells[index + 1].set;

				if (curSet != nxtSet && c.rollTheDice() > 0.5f)
				{
					cells[index].r = false;
					
					//update cells
					for (int cid : currentRow.at(nxtSet).cellIDs)
						cells[cid].set = curSet;

					//update sets
					currentRow.at(curSet).cellIDs.insert(
						currentRow.at(curSet).cellIDs.begin(), 
						currentRow.at(nxtSet).cellIDs.begin(), 
						currentRow.at(nxtSet).cellIDs.end());
					currentRow.erase(nxtSet);
				}

				//if (curSet == nxtSet) add a wall - not necessary, walls are there by default
			}
			
			//vertically connect some of the cells, but at least one per set!
			for (auto& es : currentRow)
			{
				bool connected = false;

				for (auto& cid : es.second.cellIDs)
				{
					if (c.rollTheDice() > .5f)
					{
						cells[cid].t = false;
						cells[cid + _w].set = es.first;
						connected = true;
					}
				}

				//no cells in the set were opened -> randomly pick one from the range [0, size-1] and open it to connect the set
				if (!connected)
				{
					int randIndex = floor(c.rollTheDice() * (es.second.cellIDs.size() - 1));
					cells[es.second.cellIDs[randIndex]].t = false;
					cells[es.second.cellIDs[randIndex] + _w].set = es.first;
				}	
			}
		}

		PopulateRow(_h - 1, currentRow);
		
		for (int x = 0; x < _w - 1; ++x)
		{
			int index = (_h - 1) * _w + x;
			int curSet = cells[index].set, nxtSet = cells[index + 1].set;

			if (curSet != nxtSet)
			{
				cells[index].r = false;

				//update cells
				for (int cid : currentRow.at(nxtSet).cellIDs)
					cells[cid].set = curSet;

				//update sets
				currentRow.at(curSet).cellIDs.insert(
					currentRow.at(curSet).cellIDs.begin(),
					currentRow.at(nxtSet).cellIDs.begin(),
					currentRow.at(nxtSet).cellIDs.end());
				currentRow.erase(nxtSet);
			}
		}
	}



	void Maze::PopulateRow(int z, std::map<int, EllerSet>& row)
	{

		row.clear();

		for (int x = 0; x < _w; ++x)
		{
			int index = z * _w + x;
			int setIndex = cells[index].set < 0 ? index : cells[index].set;
			cells[index] = MazeCell(setIndex, x, z);

			//set exists already in current row
			if (row.find(setIndex) != row.end())
			{
				row.at(setIndex).cellIDs.push_back(index);
			}
			else //does not exist yet in current row, therefore add it
			{
				row.insert(std::map<int, EllerSet>::value_type(setIndex, EllerSet(index)));
			}
		}
	}



	void Maze::CreateModel()
	{
		model.LoadModel("");
	}

}
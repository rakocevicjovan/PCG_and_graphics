#include "Maze.h"
#include "Geometry.h"

namespace Procedural
{

	Maze::Maze()
	{
	}



	Maze::~Maze()
	{
	}



	void Maze::Init(unsigned int w, unsigned int h, float cellSize)
	{ 
		_w = w;
		_h = h;
		_cellSize = cellSize; 

		Eller();
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



	void Maze::CreateModel(ID3D11Device* device)
	{
		Geometry g;

		float halfLength = _cellSize * .5f;
		float halfHeight = _height   * .45f;
		
		g.GenBox(SVec3(_cellSize, _height, _width));	//+_width * 0.95f
		for (auto& pos : g.positions)
		{
			pos.x += halfLength;
			pos.y += halfHeight;
		}	
		Mesh bottom = Mesh(g, device, false);

		for (auto& pos : g.positions)	pos.z += _cellSize;
		Mesh top = Mesh(g, device, false);

		g.Clear();

		g.GenBox(SVec3(_width, _height, _cellSize));	// + _width * 0.95f
		for (auto& pos : g.positions)
		{
			pos.z += halfLength;
			pos.y += halfHeight;
		}
		Mesh left = Mesh(g, device, false);

		for (auto& pos : g.positions)	pos.x += _cellSize;
		Mesh right = Mesh(g, device, false);

		for (auto& mc : cells)
			BuildCellMeshes(mc, device, left, right, top, bottom);
		
		/*
		g.Clear();

		float floorWidth = _cellSize * _w, floorHalfWidth = floorWidth * 0.5f;;
		float floorZDepth = _cellSize * _h, floorHalfZDepth = floorZDepth * 0.5f;
		float floorThiccness = 2.f;
		
		g.GenBox(SVec3(floorWidth, floorThiccness, floorZDepth));
		Mesh floor(g, device, false);

		for (auto& v : floor.vertices)
		{
			v.pos.x += floorHalfWidth;
			v.pos.z += floorHalfZDepth;
		}
		floor.setupMesh(device);
		
		model.meshes.push_back(floor);
		*/	
	}



	void Maze::BuildCellMeshes(MazeCell& mc, ID3D11Device* device, Mesh& left, Mesh& right, Mesh& top, Mesh& bottom)
	{

		if (mc.x == 0)
		{
			AlignWall(mc, left, device);
		}

		if (mc.z == 0)
		{
			AlignWall(mc, bottom, device);
		}

		if (mc.r)
		{
			AlignWall(mc, right, device);
		}

		if (mc.t)
		{
			AlignWall(mc, top, device);
		}
	}



	void Maze::AlignWall(MazeCell& mc, const Mesh& m, ID3D11Device* device)
	{
		Mesh aligned = m;

		for (auto& v : aligned.vertices)
		{
			v.pos.x += mc.x * _cellSize;
			v.pos.z += mc.z * _cellSize;
		}

		aligned.setupMesh(device);

		model.meshes.push_back(aligned);
	}
}
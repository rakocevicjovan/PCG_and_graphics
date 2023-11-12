#include "pch.h"
#include "Maze.h"
#include "Geometry.h"

namespace Procedural
{
	void Maze::Init(uint32_t w, uint32_t h, float cellSize)
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

		for (auto z = 0u; z < _h - 1; ++z)
		{
			PopulateRow(z, currentRow);

			for (auto x = 0u; x < _w - 1; ++x)
			{
				int index = z * _w + x;
				int curSet = cells[index].set, nxtSet = cells[index + 1].set;

				if (curSet != nxtSet && c.roll() > 0.5f)
				{
					cells[index].r = false;
					
					//Update cells
					for (int cid : currentRow.at(nxtSet).cellIDs)
						cells[cid].set = curSet;

					//Update sets
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
					if (c.roll() > .5f)
					{
						cells[cid].t = false;
						cells[cid + _w].set = es.first;
						connected = true;
					}
				}

				//no cells in the set were opened -> randomly pick one from the range [0, size-1] and open it to connect the set
				if (!connected)
				{
					int randIndex = static_cast<int>(floor(c.roll() * (es.second.cellIDs.size() - 1)));
					cells[es.second.cellIDs[randIndex]].t = false;
					cells[es.second.cellIDs[randIndex] + _w].set = es.first;
				}	
			}
		}

		PopulateRow(_h - 1, currentRow);
		
		for (auto x = 0u; x < _w - 1; ++x)
		{
			int index = (_h - 1) * _w + x;
			int curSet = cells[index].set, nxtSet = cells[index + 1].set;

			if (curSet != nxtSet)
			{
				cells[index].r = false;

				//Update cells
				for (int cid : currentRow.at(nxtSet).cellIDs)
					cells[cid].set = curSet;

				//Update sets
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

		for (auto x = 0u; x < _w; ++x)
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
		
		g.GenBox(SVec3(_cellSize, _height, _thickness));	//generate the wall box

		//move it along x and y to align it to maze cell's "bottom" boundary...
		//this can be solved better with instancing and matrices to reduce memory usage but the meshes are really small, plain cuboids
		for (auto& pos : g.positions)
		{
			pos.x += halfLength;
			pos.y += halfHeight;
		}	

		Mesh bottom = Mesh(g, device, false);	//bottom of the cell is now generated from modified verts

		for (auto& pos : g.positions)
			pos.z += _cellSize;

		Mesh top = Mesh(g, device, false);		//top of the cell is modified by translating them forward

		//reset geometry object to create left and right, could be rotated as well but its trivially fast for preprocessing anyways
		g.Clear();								

		g.GenBox(SVec3(_thickness, _height, _cellSize));	//generate, align to left wall
		for (auto& pos : g.positions)
		{
			pos.z += halfLength;
			pos.y += halfHeight;
		}
		Mesh left = Mesh(g, device, false);					//load vertices into mesh

		for (auto& pos : g.positions)						//move right
			pos.x += _cellSize;

		Mesh right = Mesh(g, device, false);				//load vertices into mesh

		for (auto& mc : cells)								//use meshes
			BuildCellMeshes(mc, device, left, right, top, bottom);
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

		/* // Change to fit the new vertex type
		for (auto& v : aligned._vertices)
		{
			v.pos.x += mc.x * _cellSize;
			v.pos.z += mc.z * _cellSize;
		}
		*/

		aligned.setupMesh(device);

		model._meshes.push_back(aligned);
	}

	SVec3 Maze::GetRandCellPos()
	{
		Chaos c;
		
		c.setRange(static_cast<float>(_w / 2), static_cast<float>(_w));
		uint32_t randWidth = static_cast<uint32_t>(c.roll());

		c.setRange(static_cast<float>(_h / 2), static_cast<float>(_h));
		uint32_t randDepth = static_cast<uint32_t>(c.roll());

		return SVec3((randWidth + 0.5f) * _cellSize, _height * 0.5f, (randDepth + 0.5f) * _cellSize);
	}
}
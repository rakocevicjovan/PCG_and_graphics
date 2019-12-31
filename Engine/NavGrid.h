#pragma once
#include "Math.h"
#include "NavGraphTypes.h"


struct NavCell : NavNode
{
	int _row;
	int _column;
	SVec3 _direction;
};



class NavGrid
{
private:
	int _w, _h;
	float _fw, _fh;
	SVec2 _cellSize, _invCellSize;
	SVec3 _offset;
	int _goalIndex;
	float _leeway;

public:
	std::vector<NavCell> _cells;
	std::vector<NavEdge> _edges;

	NavGrid()
	{
		_w = _h = 0;
		_fw = _fh = 0;
	}



	NavGrid(int w, int h, SVec2 cellSize, SVec3 offset = SVec3()) : _w(w), _h(h), _cellSize(cellSize), _offset(offset)
	{
		_fw = _w * _cellSize.x;
		_fh = _h * _cellSize.y;

		_invCellSize = SVec2(1.f / _cellSize.x, 1.f / _cellSize.y);

		_leeway = _cellSize.Length() * 0.5f;

		_cells.resize(_w * _h);
		_edges.reserve(2 * _w * _h - _w - _h);

		//_cells.resize(_w * _h);
		//_edges.resize(2 * _w * _h - _w - _h);
	}



	void setOffset(SVec3 offset)
	{
		_offset = offset;
	}



	//this is a naive implementation that simply adds all edges, does not check for obstacles
	void NavGrid::populate()
	{
		//create and connect edges...
		int thisCell = 0, neighbour = 0, edgeCount = 0;

		float diagonalWeight = sqrt(2);

		//wastes some but likely faster... this could fragment a lot though as _cells is a vector...
		//should use a std::array<int, 4> instead, should be much much better performance wise
		for (auto& cell : _cells)
			cell.edges.reserve(4);

		for (int i = 0; i < _h; i++)
		{
			for (int j = 0; j < _w; j++)
			{
				thisCell = i * _w + j;

				//right edge
				if (j != _w - 1)
				{
					neighbour = thisCell + 1;

					_edges.emplace_back(thisCell, neighbour);
					edgeCount++;

					_cells[thisCell].edges.push_back(edgeCount - 1);
					_cells[neighbour].edges.push_back(edgeCount - 1);
				}

				//top edge
				if (i != _h - 1)
				{
					neighbour = thisCell + _w;

					_edges.emplace_back(thisCell, neighbour);
					edgeCount++;
					
					_cells[thisCell].edges.push_back(edgeCount - 1);
					_cells[neighbour].edges.push_back(edgeCount - 1);
				}

				//top right diagonal edge
				if (i != _h - 1 && j != _w - 1)
				{
					neighbour = thisCell + _w + 1;

					_edges.emplace_back(thisCell, neighbour, diagonalWeight);
					edgeCount++;

					_cells[thisCell].edges.push_back(edgeCount - 1);
					_cells[neighbour].edges.push_back(edgeCount - 1);
				}
			}
		}
	}



	void NavGrid::fillFlowField()
	{
		for (int i = 0; i < _cells.size(); ++i)
		{
			SVec3 myPos = cellIndexToPos(i);
			float minPathCost = (std::numeric_limits<float>::max)();

			for (int j = 0; j < _cells[i].edges.size(); ++j)
			{
				const NavEdge& edge = _edges[_cells[i].edges[j]];
				int nIndex = edge.first == i ? edge.last : edge.first;
				const NavCell& neighbour = _cells[nIndex];
				float curPathCost = neighbour.pathWeight;

				if (neighbour.pathWeight < minPathCost)
				{
					minPathCost = neighbour.pathWeight;
					_cells[i]._direction = cellIndexToPos(nIndex) - myPos;
					_cells[i]._direction.Normalize();
				}
			}
		}
	}


	
	SVec3 snapToCell(const SVec3& pos)
	{
		SVec3 result;

		SVec3 adjPos = pos - _offset;

		int row = floor(adjPos.z * _invCellSize.y);
		int column = floor(adjPos.x * _invCellSize.x);

		//SVec2 offset = pos - SVec2(row * _cellSize.x, column * _cellSize.y);

		result = cellIndexToPos(row * _w + column);

		return result;
	}



	int posToCell(SVec3 pos) const
	{
		SVec3 offsetFromGrid = pos - _offset;

		offsetFromGrid.x = Math::clamp(0, _fw - .01f, offsetFromGrid.x);
		offsetFromGrid.z = Math::clamp(0, _fh - .01f, offsetFromGrid.z);

		int row = floor(offsetFromGrid.z * _invCellSize.y);
		int column = floor(offsetFromGrid.x * _invCellSize.x);
		return row * _w + column;
	}


	SVec3 cellIndexToPos(int i) const
	{
		int row = i % _w;
		int column = floor(i / _w);

		SVec3 posInGrid(row * _cellSize.x, 0, column * _cellSize.y);
		SVec3 cellCenterOffset(_cellSize.x * .5f, 0, _cellSize.y * .5f);

		return _offset + posInGrid + cellCenterOffset;
	}


	inline SVec3 flowAtIndex(int i) const
	{
		return _cells[i]._direction;
	}


	inline SVec3 flowAtPosition(SVec3 pos) const
	{
		return _cells[posToCell(pos)]._direction;
	}


	inline SVec2 getCellSize() const
	{
		return _cellSize;
	}


	inline int getGoalIndex() const
	{
		return _goalIndex;
	}


	inline void setGoalIndex(int gi)
	{
		_goalIndex = gi;
	}


	inline float getLeeway() const
	{
		return _leeway;
	}


	inline void setLeeway(float leeway)
	{
		_leeway = leeway;
	}
};
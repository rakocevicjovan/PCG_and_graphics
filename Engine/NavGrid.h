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
	SVec2 _cellSize;
	SVec3 _offset;

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

		_cells.reserve(_w * _h);
		_edges.reserve(2 * _w * _h - _w - _h);

		//_cells.resize(_w * _h);
		//_edges.resize(2 * _w * _h - _w - _h);
	}


	void setOffset(SVec3 offset)
	{
		_offset = offset;
	}


	void NavGrid::createEdges()
	{
		//create and connect edges...
		int thisCell = 0, neighbour = 0, edgeCount = 0;

		//wastes some but likely faster... this could fragment a lot though as _cells is a vector...
		//should use a std::array<int, 4> instead, should be much much better performance wise
		for (auto& cell : _cells)
			cell.edges.reserve(4);

		for (int i = 0; i < _h; i++)
		{
			for (int j = 0; j < _w; j++)
			{
				//right edge
				if (j != _w - 1)
				{
					thisCell = i * _w + _h;
					neighbour = thisCell + 1;

					_edges.emplace_back(thisCell, neighbour);
					edgeCount++;

					_cells[thisCell].edges.push_back(edgeCount - 1);
					_cells[neighbour].edges.push_back(edgeCount - 1);
				}

				//top edge
				if (i != _h - 1)
				{
					thisCell = i * _w + _h;
					neighbour = thisCell + _w;

					_edges.emplace_back(thisCell, neighbour);
					edgeCount++;
					
					_cells[thisCell].edges.push_back(edgeCount - 1);
					_cells[neighbour].edges.push_back(edgeCount - 1);
				}
			}
		}



	}
};
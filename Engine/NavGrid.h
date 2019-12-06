#pragma once
#include "Math.h"


class NavGrid
{
private:
	class NavCell
	{
		int row;
		int column;
	};

	int _w, _h;
	float _fw, _fh;
	SVec2 _cellSize;

	NavGrid(int w, int h, SVec2 cellSize) : _w(w), _h(h), _cellSize(cellSize)
	{
		_fw = _w * _cellSize.x;
		_fh = _h * _cellSize.y;
	}

};
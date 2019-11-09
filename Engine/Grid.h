#pragma once

#include <map>
#include <set>
#include "Math.h"

class Hull;
class AABB;
class SphereHull;


struct CellKey
{
	int x = 0, y = 0, z = 0;

	CellKey(int ix, int iy, int iz) : x(ix), y(iy), z(iz) {};

	CellKey() : x(-1), y(-1), z(-1) {};


#define FASTFLOOR(x) ( (x >= 0.f) ? ((int)x) : ((int)x-1 ) )

	CellKey(const SVec3& pos, float invCellSize)
	{
		x = FASTFLOOR(pos.x * invCellSize);
		y = FASTFLOOR(pos.y * invCellSize);
		z = FASTFLOOR(pos.z * invCellSize);
	}

#undef FASTFLOOR

	inline void assign(const SVec3& in) { x = floor(in.x); y = floor(in.y); z = floor(in.z); }

	int total() const { return x * y * z + y * z + z; }

	bool operator <(const CellKey& other) const { return total() < other.total(); }
};



struct GridCell
{
	std::set<Hull*> hulls;
};



class Grid
{
public:
	Grid() {};
	Grid(float cellsize) : _cellsize(cellsize) {}
	float _cellsize = 32.f;
	float invCellSize = 1.f / _cellsize;
	std::map<CellKey, GridCell> cells;

	void addAABB(AABB* h);
	void addSphere(SphereHull* h);
	//void removeAABB(Hull *h);
	//void removeSphere(Hull* h);
};

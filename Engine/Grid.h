#pragma once

#include "Math.h"

class Hull;
class AABB;
class SphereHull;


struct CellKey
{
	int x = 0;
	int y = 0;
	int z = 0;

	CellKey(int ix, int iy, int iz) : x(ix), y(iy), z(iz) {};

	CellKey() : x(-1), y(-1), z(-1) {};

	CellKey(const SVec3& pos, float invCellSize)
	{
		const auto xyz = Math::floor3(pos * invCellSize);
		x = static_cast<int>(xyz.x);
		y = static_cast<int>(xyz.y);
		z = static_cast<int>(xyz.z);
	}

	inline void assign(const SVec3& in)
	{ 
		x = static_cast<int>(floor(in.x)); 
		y = static_cast<int>(floor(in.y)); 
		z = static_cast<int>(floor(in.z));
	}

	int total() const 
	{ 
		return x * y * z + y * z + z;
	}

	bool operator <(const CellKey& other) const 
	{ 
		return total() < other.total(); 
	}
};


struct GridCell
{
	std::set<Hull*> _hulls;
};


class Grid
{
public:
	Grid() {};
	Grid(float cellsize) : _cellsize(cellsize) {}
	float _cellsize = 32.f;
	float invCellSize = 1.f / _cellsize;
	std::map<CellKey, GridCell> cells;	//@TODO this is absolutely not okay for cache locality, change it... will take some work

	void addAABB(AABB* h);
	void addSphere(SphereHull* h);
	//void removeAABB(Hull *h);
	//void removeSphere(Hull* h);
};

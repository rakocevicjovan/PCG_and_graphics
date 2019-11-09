#pragma once
#include "Grid.h"
#include "Hull.h"


void Grid::addAABB(AABB* h)
{
	std::vector<SVec3> positions = h->getVertices();

	CellKey minCellKey(h->minPoint, invCellSize);
	CellKey maxCellKey(h->maxPoint, invCellSize);

	for (int i = minCellKey.x; i <= maxCellKey.x; ++i)
		for (int j = minCellKey.y; j <= maxCellKey.y; ++j)
			for (int k = minCellKey.z; k <= maxCellKey.z; ++k)
				cells[CellKey(i, j, k)].hulls.insert(h);
}


//@TODO...
void Grid::addSphere(SphereHull* h)
{

}
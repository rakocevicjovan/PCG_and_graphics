#include "Hexer.h"


void Hexer::init(float cellSize, SVec3 rootPos)
{
	_cellSize = cellSize;
	_cellDist = sqrt(3) * _cellSize;
	_triHeight = _cellDist * .5f;

	SMatrix mat;
	mat = SMatrix::CreateFromAxisAngle(SVec3(0, 1, 0), PI * .333333f);

	SVec3 dir(0, 0, 1);
	dir = SVec3::Transform(dir, SMatrix::CreateFromAxisAngle(SVec3(0, 1, 0), PI * .166667f));
	dir *= _cellDist;

	for (int i = 0; i < 6; ++i)
	{
		Math::RotateVecByMat(dir, mat);
		dirs.emplace_back(dir);
	}

	_lastPlatformPos = rootPos;
	_platforms.emplace_back(rootPos);
}



void Hexer::addPlatform(SVec3 parentPos, int dirIndex)
{
	_platforms.emplace_back(parentPos + dirs[dirIndex]);
}



void Hexer::update(float dTime)
{
	for (auto& p : _platforms)
	{
		if (!p.active) continue;

		if (p.age >= _lifeSpan)
			p.active = false;

		p.age += dTime;
	}
	//platforms.erase(std::remove_if(platforms.begin(), platforms.end(), [&](const Platform& p) { return p.age >= _lifeSpan; }), platforms.end());
}



SVec3 Hexer::getCornerPos(const SVec3& center, UINT i)
{
	float degrees = 60.f * i - 30.f;
	float radians = PI / 180.f * degrees;
	return SVec3(center.x + _cellSize * cos(radians), center.y, center.z + _cellSize * sin(radians));
}



void Hexer::createObstacleCourse()
{
	std::vector<SVec3> rests;
	SVec3 finalPlatform;

}



AxialCoords Hexer::cubeToAxial(CellKey ck)
{
	return AxialCoords(ck.x, ck.z);
}



CellKey Hexer::axialToCube(AxialCoords ac)
{
		return CellKey(ac.q, -ac.q - ac.r, ac.r)
}

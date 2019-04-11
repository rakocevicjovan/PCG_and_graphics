#include "Hexer.h"
#include "Chaos.h"
#include "Systems.h"




Platform::Platform(SVec3 position, Model* m, ShaderMaze* s)
{
	actor.transform = SMatrix::CreateTranslation(position);
	actor.gc.model = m;
	actor.gc.shader = s;
}



void Hexer::init(float cellSize)
{
	_cellSize = cellSize;
	_cellDist = sqrt(3) * _cellSize;
	_triHeight = _cellDist * .5f;

	_lastPlatformPos = _points[0];

	SMatrix mat;
	mat = SMatrix::CreateFromAxisAngle(SVec3(0, 1, 0), PI * .333333f);

	SVec3 point(0, 0, 1);
	point = SVec3::Transform(point, SMatrix::CreateFromAxisAngle(SVec3(0, 1, 0), PI * .166667f));	//shift 30 degrees initially
	point *= _cellDist;	//offset by distance to point

	for (int i = 0; i < 6; ++i)
	{
		Math::RotateVecByMat(point, mat);	//keep shifting 60 degrees
		_edgeNormals.emplace_back(point);
	}
}



SVec3 Hexer::calcPlatformPos(SVec3 parentPos, int dirIndex)
{
	SVec3 newPos = parentPos + _edgeNormals[dirIndex];
	_lastPlatformPos = newPos;
	sincePlatformAdded = 0;
	return newPos;
}



void Hexer::update(float dTime)
{
	sincePlatformAdded += dTime;

	for (auto& p : _platforms)
	{
		p.age += dTime;

		if (p.age >= _lifeSpan)
		{
			p.active = false;
		}
	}
	_platforms.erase(std::remove_if(_platforms.begin(), _platforms.end(), [&](const Platform& p) { return !p.active; }), _platforms.end());
}


bool Hexer::marchTowardsPoint(SVec3& newPlatformPos)
{
	if (done || sincePlatformAdded < platformSpawnRate)
		return false;

	SVec3 heading = _points[targetIndex] - _lastPlatformPos;
	std::vector<int> facingTarget;

	for (int i = 0; i < _edgeNormals.size(); ++i)
	{
		SVec3 newPos = _lastPlatformPos + _edgeNormals[i];
		
		//reject duplicates
		bool duplicate = false;
		for (auto&p : previousPositions)
		{
			if (SVec3::DistanceSquared(newPos, p) < 1.f)
			{
				duplicate = true;
				break;
			}
		}

		if (heading.Dot(_edgeNormals[i]) > 0.f && !duplicate)
			facingTarget.push_back(i);
	}
	
	Chaos c;
	int neighbourPos = facingTarget[c.rollTheDice() * (facingTarget.size() - 1)];

	if (SVec3::DistanceSquared(_lastPlatformPos, _points[targetIndex]) < _cellSize * _cellSize)
	{
		++targetIndex;
		if (targetIndex >= _points.size())
			done = true;
	}

	newPlatformPos = calcPlatformPos(_lastPlatformPos, neighbourPos);

	if (previousPositions.size() >= 6)
		previousPositions.pop_front();

	previousPositions.push_back(_lastPlatformPos);

	return true;
}



SVec3 Hexer::getCornerPos(const SVec3& center, UINT i)
{
	float degrees = 60.f * i - 30.f;
	float radians = PI / 180.f * degrees;
	return SVec3(center.x + _cellSize * cos(radians), center.y, center.z + _cellSize * sin(radians));
}














/*


struct CubeCoords
{
	CubeCoords() {};
	CubeCoords(int x, int y, int z) : x(x), y(y), z(z) {}

	static CubeCoords cubeAdd(CubeCoords c1, CubeCoords c2) { return CubeCoords(c1.x + c2.x, c1.y + c2.y, c1.z + c2.z); }
	static std::vector<CubeCoords> cubeDirs;


	int x, y, z;
};
struct AxialCoords
{
	AxialCoords() {}
	AxialCoords(int q, int r) : q(q), r(r) {}

	static std::vector<AxialCoords> axialCoords;

	int q, r;
};
struct HexGridCell
{
	CubeCoords ck;
};


static std::vector<CubeCoords> cubeDirs = {
	CubeCoords(+1, -1, 0), CubeCoords(+1, 0, -1), CubeCoords(0, +1, -1), CubeCoords(-1, +1, 0), CubeCoords(-1, 0, +1), CubeCoords(0, -1, +1)
};

static std::vector<AxialCoords> axialDirs = {
	AxialCoords(+1, 0), AxialCoords(+1, -1), AxialCoords(0, -1), AxialCoords(-1, 0), AxialCoords(-1, +1), AxialCoords(0, +1)
};



AxialCoords Hexer::cubeToAxial(CubeCoords ck)
{
	return AxialCoords(ck.x, ck.z);
}



CubeCoords Hexer::axialToCube(AxialCoords ac)
{
	return CubeCoords(ac.q, -ac.q - ac.r, ac.r);
}



CubeCoords Hexer::getCubeDir(UINT direction)
{
	return cubeDirs[direction];
}
	


CubeCoords Hexer::getNeighbourAtDirection(CubeCoords cube, UINT direction)
{
	return CubeCoords::cubeAdd(cube, getCubeDir(direction));
}



float cubeDistance(CubeCoords a, CubeCoords b)
{
	return (abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z)) * 0.5f;	//half of manhattan distance because we can take diagonals if we need? not sure
}



CubeCoords axialToCube(AxialCoords ac);
AxialCoords cubeToAxial(CubeCoords ck);
CubeCoords getCubeDir(UINT direction);
CubeCoords getNeighbourAtDirection(CubeCoords cube, UINT direction);
*/
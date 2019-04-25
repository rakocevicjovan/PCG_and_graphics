#pragma once
#include "Geometry.h"
#include "Math.h"
#include "GameObject.h"
#include <deque>
#include <list>

class Systems;

struct Platform
{
	Platform() {}

	Platform(SVec3 position, Model* m, ShaderMaze* s);

	Actor actor;
	float age = 0.f;
	bool active = true;
};

class Hexer
{
	Systems& _sys;
	std::vector<SVec3> _edgeNormals;
	std::deque<SVec3> previousPositions;

	UINT targetIndex = 1;
	float _lifeSpan = 3.33f, _cellSize = 0.f, _cellDist = 0.f, _triHeight = 0.f, 
		platformSpawnRate = _lifeSpan * 0.666666f, sincePlatformAdded = 0.f;
	bool done = false;

public:

	
	Hexer(Systems& sys) : _sys(sys) {};
	~Hexer() {}

	std::list<Platform> _platforms;
	SVec3 _lastPlatformPos;

	void init(float cellSize);
	SVec3 calcPlatformPos(SVec3 parentPos, int direction);
	void update(float dTime);
	bool marchTowardsPoint(SVec3& newPlatformPos);

	SVec3 getCornerPos(const SVec3& center, UINT i);

	std::vector<SVec3> _points = {
		SVec3(190, 128, 185), SVec3(408, 128, 308),
		SVec3(417, 128, 570), SVec3(243, 128, 782), SVec3(768, 128, 782)
	};
};
#pragma once
#include "Geometry.h"
#include "Math.h"
#include "GameObject.h"
#include <deque>

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
	float _lifeSpan = 2.66f, _cellSize = 0.f, _cellDist = 0.f, _triHeight = 0.f;
	bool done = false;

public:

	float sincePlatformAdded = 0.f, platformSpawnRate = _lifeSpan * 0.666666f;
	
	Hexer(Systems& sys) : _sys(sys) {};
	~Hexer() {}

	std::vector<Platform> _platforms;
	SVec3 _lastPlatformPos;

	void init(float cellSize);
	SVec3 calcPlatformPos(SVec3 parentPos, int direction);
	void update(float dTime);
	bool marchTowardsPoint(SVec3& newPlatformPos);

	SVec3 getCornerPos(const SVec3& center, UINT i);

	std::vector<SVec3> _points = {
		SVec3(190, 128, 185), SVec3(508, 128, 208),
		SVec3(417, 128, 570), SVec3(243, 128, 782),
		SVec3(675, 128, 777), SVec3(768, 128, 782)
	};
};
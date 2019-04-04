#include "Hexer.h"


void Hexer::init()
{
	SMatrix m;
	m.CreateFromAxisAngle(SVec3(0, 1, 0), PI * .333333f);

	SVec3 dir(cos(PI / 6), 0, sin(PI / 6));
	dir *= cellSize;

	for (int i = 0; i < 6; ++i)
	{
		Math::RotateVecByMat(dir, m);
		dirs.emplace_back(dir);
	}

	root.position = SVec3(0, 0, 0);
}



void Hexer::addPlatform(Platform p, int dirIndex)
{
	SVec3 offset = dirs[dirIndex];
	platforms.emplace_back(p.position + offset);
}



void Hexer::update(float dTime)
{
	for (auto& p : platforms)
		p.age += dTime;

	platforms.erase(std::remove_if(platforms.begin(), platforms.end(), [&](const Platform& p) { return p.age >= _lifeSpan; }), platforms.end());
}

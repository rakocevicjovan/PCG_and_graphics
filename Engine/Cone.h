#pragma once
#include "Math.h"

class Cone
{
public:
	SVec3 _tip;
	float _r;

	SVec3 _dir;
	float _d;

	Cone();
	Cone(const SVec3& tip, float r, const SVec3& dir, float d) : _tip(tip), _r(r), _dir(dir), _d(d) {}

	Cone(const SVec3& tip, const SVec3& dir, float length, float halfAngle) 
		: _tip(tip), _dir(dir), _d(length), _r(length * tan(halfAngle)) {}
};
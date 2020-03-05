#pragma once
#include "Math.h"

class Cone
{
public:
	SVec3 _tip;
	float _radius;

	SVec3 _dir;
	float _d;

	Cone(const SVec3& tip, float r, const SVec3& dir, float d) 
		: _tip(tip), _radius(r), _dir(dir), _d(d) {}

	Cone(const SVec3& tip, const SVec3& dir, float length, float halfAngle) 
		: _tip(tip), _radius(length * tan(halfAngle)), _dir(dir), _d(length) {}

	// Quick construction from spotlight (precalculated tangent)
	Cone(const SVec4& posRange, const SVec3& dir, float radius)
		: _tip(SVec3(posRange)), _radius(radius), _dir(dir), _d(posRange.w) {}
};
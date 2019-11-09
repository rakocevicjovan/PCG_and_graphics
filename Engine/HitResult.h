#pragma once
#include "Math.h"

class Collider;

struct HitResult
{
	bool hit = false;
	SVec3 resolutionVector = SVec3();
	float sqPenetrationDepth = 0.f;
	Collider* c;

	HitResult() {}
	HitResult(bool h, SVec3 rv, float sqpd) : hit(h), resolutionVector(rv), sqPenetrationDepth(sqpd) {}
};
#pragma once
#include "Math.h"

class Collider;

struct HitResult
{
	bool hit = false;
	SVec3 resolutionVector = SVec3::Zero;
	float sqPenetrationDepth = 0.f;

	HitResult() : hit(false), resolutionVector(SVec3::Zero), sqPenetrationDepth(0.f) {}
	HitResult(bool h, SVec3 rv, float sqpd) : hit(h), resolutionVector(rv), sqPenetrationDepth(sqpd) {}
};
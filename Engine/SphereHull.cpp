/*
#include "SphereHull.h"
#include "ColFuncs.h"

HitResult SphereHull::intersect(const Hull* other, BoundingVolumeType otherType) const
{
	if (otherType == BVT_SPHERE)
		return Col::SphereSphereIntersection(*this, *(reinterpret_cast<const SphereHull*>(other)));
	else if (otherType == BVT_AABB)
		return Col::AABBSphereIntersection(*(reinterpret_cast<const AABB*>(other)), *this);

	return HitResult();
}
*/
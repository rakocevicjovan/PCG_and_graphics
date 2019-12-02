#include "Collider.h"
#include "ColFuncs.h"

//lmao just use templates fuck this, what was I thinking
HitResult Collider::Collide(const Collider& other, SVec3& resolutionVector)
{
	HitResult hitRes;

	for (Hull* hull1 : hulls)
	{
		for (Hull* hull2 : other.hulls)
		{
			if (BVT == BVT_AABB)
				hitRes = reinterpret_cast<AABB*>(hull1)->intersect(hull2, other.BVT);
			if (BVT == BVT_SPHERE)
				hitRes = reinterpret_cast<SphereHull*>(hull1)->intersect(hull2, other.BVT);

			if (hitRes.hit)
			{
				resolutionVector = hull2->getPosition() - hull1->getPosition();
				resolutionVector.Normalize();
				break;
			}
		}
	}

	return hitRes;
}
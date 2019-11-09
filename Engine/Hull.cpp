#include "Hull.h"
#include "Collider.h"

HitResult AABB::intersect(const Hull* other, BoundingVolumeType otherType) const
{
	if (otherType == BVT_SPHERE)		return Collider::AABBSphereIntersection(*this, *(reinterpret_cast<const SphereHull*>(other)));
	else if (otherType == BVT_AABB)		return Collider::AABBAABBIntersection(*this, *(reinterpret_cast<const AABB*>(other)));
	return HitResult();
}



HitResult SphereHull::intersect(const Hull* other, BoundingVolumeType otherType) const
{
	if (otherType == BVT_SPHERE)		return Collider::SphereSphereIntersection(*this, *(reinterpret_cast<const SphereHull*>(other)));
	else if (otherType == BVT_AABB)		return Collider::AABBSphereIntersection(*(reinterpret_cast<const AABB*>(other)), *this);
	return HitResult();
}



std::vector<SVec3> AABB::getVertices() const
{
	return
	{
		minPoint,						//left  lower  near
		SVec3(minPoint.x, minPoint.y, maxPoint.z),	//left  lower  far
		SVec3(maxPoint.x, minPoint.y, minPoint.z),	//right lower  near
		SVec3(maxPoint.x, minPoint.y, maxPoint.z),	//right lower  far
		maxPoint,						//right higher far
		SVec3(minPoint.x, maxPoint.y, maxPoint.z),	//left  higher far
		SVec3(maxPoint.x, maxPoint.y, minPoint.z),	//right higher near
		SVec3(minPoint.x, maxPoint.y, minPoint.z),	//left  higher near
	};
}



std::vector<SPlane> AABB::getPlanes() const
{
	//b t n f l r
	std::vector<SVec3> v = getVertices();
	std::vector<SPlane> result;
	result.reserve(6);

	result.emplace_back(v[0], v[1], v[2]);	//bottom
	result.emplace_back(v[4], v[5], v[6]);	//top
	result.emplace_back(v[0], v[2], v[6]);	//near
	result.emplace_back(v[1], v[3], v[4]);	//far
	result.emplace_back(v[0], v[1], v[5]);	//left
	result.emplace_back(v[2], v[3], v[4]);	//right

	return result;
}
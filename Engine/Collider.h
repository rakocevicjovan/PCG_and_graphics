#pragma once
#include "Hull.h"

class Actor;
class Model;
class Mesh;


class Collider
{
public:
	BoundingVolumeType BVT;
	Model* modParent;
	Actor* actParent;
	SMatrix transform;
	std::vector<Hull*> hulls;
	bool dynamic;


	Collider() {}
	Collider(BoundingVolumeType type, Model* m, std::vector<Hull*> hullptrs) : BVT(type), modParent(m), hulls(hullptrs) {}
	Collider(BoundingVolumeType type, Actor* a, std::vector<Hull*> hullptrs) : BVT(type), actParent(a), hulls(hullptrs) {}

	~Collider() { }

	void ReleaseMemory()
	{
		for (Hull* hull : hulls)
		{
			delete hull;
			hull = nullptr;
		}
	}

	bool operator ==(const Collider& other) const
	{
		if (dynamic == true)
			return actParent == other.actParent;
		else
			return modParent == other.modParent;
	}

	static HitResult AABBSphereIntersection(const AABB& b, const SphereHull& s);
	static HitResult SphereSphereIntersection(const SphereHull& s1, const SphereHull& s2);
	static HitResult AABBAABBIntersection(const AABB& a, const AABB& b);
	static bool RaySphereIntersection(const SRay& ray, const SphereHull& s);
	static HitResult RayAABBIntersection(const SRay& ray, const AABB& b, SVec3& poi, float& t);
	static float ClosestPointOnAABB(SVec3 p, AABB b, SVec3& out);

	static bool RayPlaneIntersection(const SRay& ray, const SVec3& a, const SVec3& b, const SVec3& c, SVec3& intersectionPoint);
	static bool RayTriangleIntersection(const SRay& ray, const SVec3& a, const SVec3& b, const SVec3& c);

	HitResult Collider::Collide(const Collider& other, SVec3& resolutionVector);
};
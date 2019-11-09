#include "Collider.h"



HitResult Collider::Collide(const Collider& other, SVec3& resolutionVector)
{
	HitResult hitRes;

	for (Hull* hull1 : hulls)
	{
		for (Hull* hull2 : other.hulls)
		{
			if (BVT == BVT_AABB)		hitRes = reinterpret_cast<AABB*>(hull1)->intersect(hull2, other.BVT);
			if (BVT == BVT_SPHERE)	hitRes = reinterpret_cast<SphereHull*>(hull1)->intersect(hull2, other.BVT);

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



//ACTUAL INTERSECTION CODE AFTER ALL THIS PAPERWORK
inline float sq(float x) { return x * x; }

float Collider::ClosestPointOnAABB(SVec3 p, AABB b, SVec3& out)
{
	out.x = Math::clamp(b.minPoint.x, b.maxPoint.x, p.x);
	out.y = Math::clamp(b.minPoint.y, b.maxPoint.y, p.y);
	out.z = Math::clamp(b.minPoint.z, b.maxPoint.z, p.z);

	return SVec3::DistanceSquared(p, out);
}


//resolution vector is the vector from the sphere center to the closest point
HitResult Collider::AABBSphereIntersection(const AABB& b, const SphereHull& s)
{
	HitResult hr;
	SVec3 closestPointOnAABB;

	float sqdToClosestPoint = ClosestPointOnAABB(s.c, b, closestPointOnAABB);
	float sqpenetrationDepth = sq(s.r) - sqdToClosestPoint;

	hr.hit = sqpenetrationDepth > 0;
	hr.sqPenetrationDepth = hr.hit ? sqpenetrationDepth : 0.f;

	SVec3 resVec = s.c - closestPointOnAABB;	//if sphere is in the object this will be 0...

	if (resVec.LengthSquared() < 0.0001f)
		hr.resolutionVector = Math::getNormalizedVec3(closestPointOnAABB - b.getPosition());
	else
		hr.resolutionVector = Math::getNormalizedVec3(resVec);

	return hr;
}



HitResult Collider::SphereSphereIntersection(const SphereHull& s1, const SphereHull& s2)
{
	float distSquared = SVec3::DistanceSquared(s1.c, s2.c);
	return HitResult(distSquared < sq(s1.r + s2.r), s1.c - s2.c, distSquared);
}



HitResult Collider::AABBAABBIntersection(const AABB& a, const AABB& b)
{
	if (a.maxPoint.x < b.minPoint.x || a.minPoint.x > b.maxPoint.x) return HitResult();
	if (a.maxPoint.y < b.minPoint.y || a.minPoint.y > b.maxPoint.y) return HitResult();
	if (a.maxPoint.z < b.minPoint.z || a.minPoint.z > b.maxPoint.z) return HitResult();
	return HitResult(true, a.getPosition() - b.getPosition(), 0.f);
}


//not implemented @TODO
bool Collider::RaySphereIntersection(const SRay& ray, const SphereHull& s)
{
	return false;
}



bool ClipLine(int dim, const AABB& b, const SRay& lineSeg, float& lo, float& hi)
{
	float fDimLow, fDimHigh;
	float inv = 1.f / (lineSeg.direction.at(dim) - lineSeg.position.at(dim));

	fDimLow = (b.minPoint.at(dim) - lineSeg.position.at(dim)) * inv;
	fDimHigh = (b.maxPoint.at(dim) - lineSeg.position.at(dim)) * inv;

	if (fDimHigh < fDimLow)
		Math::swap(fDimHigh, fDimLow);

	if (fDimHigh < lo) return false;
	if (fDimLow > hi) return false;

	lo = max(fDimLow, lo);
	hi = max(fDimHigh, hi);

	return (lo < hi);
}



//ray structure used for convenience, it's actually first and last point of the line
HitResult Collider::RayAABBIntersection(const SRay& lineSeg, const AABB& b, SVec3& poi, float& t)
{
	HitResult hr;

	SVec3 lineDirection = lineSeg.direction - lineSeg.position;

	float invDeltaX = 1.f / (lineDirection.x);
	float invDeltaY = 1.f / (lineDirection.y);
	float invDeltaZ = 1.f / (lineDirection.z);

	float flow = 0.f;
	float fhigh = 1.f;

	for (int i = 0; i < 3; ++i)
	{
		hr.hit = ClipLine(i, b, lineSeg, flow, fhigh);
		if (!hr.hit) return hr;
	}

	poi = lineSeg.position + lineDirection * flow;
	t = flow;

	return hr;
}



bool Collider::RayPlaneIntersection(const SRay& ray, const SVec3& a, const SVec3& b, const SVec3& c, SVec3& intersectionPoint)
{
	SVec3 normal = (b - a).Cross(c - b);

	float t = (c - ray.position).Dot(normal) / ray.direction.Dot(normal);

	if (t > 0 && t < 1)
	{
		intersectionPoint = ray.position + t * ray.direction;
		return true;
	}
	return false;
}



bool Collider::RayTriangleIntersection(const SRay& ray, const SVec3& a, const SVec3& b, const SVec3& c)
{
	SVec3 i;
	if (!RayPlaneIntersection(ray, a, b, c, i)) return false;

	SVec3 ab = b - a;
	SVec3 cb = b - c;
	SVec3 ac = c - a;

	SVec3 projABOntoCB = Math::projectVecOntoVec(ab, cb);
	SVec3 v = ab - projABOntoCB;

	SVec3 ai = i - a;

	float aBar = 1 - (v.Dot(ai) / v.Dot(ab));

	if (aBar < 0.f || aBar > 1.f)
		return false;

	SVec3 projABontoAC = Math::projectVecOntoVec(ab, ac);
	v = -ab + projABontoAC;
	SVec3 bi = i - b;

	float bBar = 1 - (v.Dot(bi) / v.Dot(-ab));

	return bBar > 0.f && bBar < 1.f;
}
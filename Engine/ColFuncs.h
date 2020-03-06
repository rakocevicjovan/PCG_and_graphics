#pragma once
#include "Hull.h"
//#include "AABB.h"
//#include "SphereHull.h"
#include "Math.h"
#include "Frustum.h"

/* The "header"
static HitResult AABBSphereIntersection(const AABB& b, const SphereHull& s);
static HitResult SphereSphereIntersection(const SphereHull& s1, const SphereHull& s2);
static HitResult AABBAABBIntersection(const AABB& a, const AABB& b);
static bool RaySphereIntersection(const SRay& ray, const SphereHull& s);
static HitResult RayAABBIntersection(const SRay& ray, const AABB& b, SVec3& poi, float& t);
static float ClosestPointOnAABB(SVec3 p, AABB b, SVec3& out);
static bool RayPlaneIntersection(const SRay& ray, const SVec3& a, const SVec3& b, const SVec3& c, SVec3& intersectionPoint);
static bool RayTriangleIntersection(const SRay& ray, const SVec3& a, const SVec3& b, const SVec3& c);
*/

inline float sq(float x) { return x * x; }

namespace Col
{

	static float ClosestPointOnAABB(SVec3 p, AABB b, SVec3& out)
	{
		out.x = Math::clamp(b.minPoint.x, b.maxPoint.x, p.x);
		out.y = Math::clamp(b.minPoint.y, b.maxPoint.y, p.y);
		out.z = Math::clamp(b.minPoint.z, b.maxPoint.z, p.z);

		return SVec3::DistanceSquared(p, out);
	}


	//resolution vector is the vector from the sphere center to the closest point
	static HitResult AABBSphereIntersection(const AABB& b, const SphereHull& s)
	{
		HitResult hr;
		SVec3 closestPointOnAABB;

		float sqdToClosestPoint = ClosestPointOnAABB(s.ctr, b, closestPointOnAABB);
		float sqPenetrationDepth = sq(s.r) - sqdToClosestPoint;

		hr.hit = sqPenetrationDepth > 0;
		hr.sqPenetrationDepth = hr.hit ? sqPenetrationDepth : 0.f;

		SVec3 resVec = s.ctr - closestPointOnAABB;	//if sphere is in the object this will be 0...

		if (resVec.LengthSquared() < 0.0001f)
			hr.resolutionVector = Math::getNormalizedVec3(closestPointOnAABB - b.getPosition());
		else
			hr.resolutionVector = Math::getNormalizedVec3(resVec);

		return hr;
	}


	inline static bool AABBSphereSimpleIntersection(const AABB& b, const SphereHull& s)
	{
		SVec3 closestPointOnAABB;

		float sqdToClosestPoint = ClosestPointOnAABB(s.ctr, b, closestPointOnAABB);
		
		return (sq(s.r) > sqdToClosestPoint);
	}



	static HitResult SphereSphereIntersection(const SphereHull& s1, const SphereHull& s2)
	{
		float distSquared = SVec3::DistanceSquared(s1.ctr, s2.ctr);

		if (distSquared < 0.001f)
		{
			return HitResult(true, SVec3(1), s1.r*s2.r);
		}

		return HitResult(distSquared < sq(s1.r + s2.r), s1.ctr - s2.ctr, distSquared);
	}



	static HitResult AABBAABBIntersection(const AABB& a, const AABB& b)
	{
		if (a.maxPoint.x < b.minPoint.x || a.minPoint.x > b.maxPoint.x) return HitResult();
		if (a.maxPoint.y < b.minPoint.y || a.minPoint.y > b.maxPoint.y) return HitResult();
		if (a.maxPoint.z < b.minPoint.z || a.minPoint.z > b.maxPoint.z) return HitResult();
		return HitResult(true, a.getPosition() - b.getPosition(), 0.f);
	}



	static bool RaySphereIntersection(const SRay& ray, const SphereHull& s)
	{
		SVec3 projectedPoint = Math::projectVecOntoVec(s.ctr - ray.position, ray.direction);

		//float t = projectedPoint.Length();

		SVec3 closestPointOnRay = ray.position + projectedPoint;
		float sqDistToRay = (closestPointOnRay - s.ctr).LengthSquared();
		float sqRadius = sq(s.r);

		return (sqDistToRay < sqRadius);
	}



	static bool ClipLine(int dim, const AABB& b, const SRay& lineSeg, float& lo, float& hi)
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
	static HitResult LSegmentAABBIntersection(const SRay& lineSeg, const AABB& b, SVec3& poi, float& t)
	{
		HitResult hr;

		SVec3 lineDirection = lineSeg.direction - lineSeg.position;

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



	static bool LSegmentAABBSimpleIntersection(const SRay& lineSeg, const AABB& b)
	{
		SVec3 lineDirection = lineSeg.direction - lineSeg.position;

		float flow = 0.f;
		float fhigh = 1.f;

		for (int i = 0; i < 3; ++i)
			if (!ClipLine(i, b, lineSeg, flow, fhigh))
				return false;

		return true;
	}



	static bool RayPlaneIntersection(const SRay& ray, const SVec3& a, const SVec3& b, const SVec3& c, SVec3& intersectionPoint)
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



	static bool RayTriangleIntersection(const SRay& ray, const SVec3& a, const SVec3& b, const SVec3& c)
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


	static bool IntersectRaySphere(SVec3 p, SVec3 d, SphereHull s, float &t, SVec3 &q)
	{
		SVec3 m = p - s.ctr;
		float b = m.Dot(d);
		float c = m.Dot(m) - s.r * s.r;

		// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0) 
		if (c > 0.0f && b > 0.0f) return 0;
		float discr = b * b - c;

		// A negative discriminant corresponds to ray missing sphere 
		if (discr < 0.0f) return 0;

		// Ray now found to intersect sphere, compute smallest t value of intersection
		t = -b - sqrt(discr);

		// If t is negative, ray started inside sphere so clamp t to zero 
		if (t < 0.0f) t = 0.0f;
		q = p + t * d;

		return 1;
	}



	static bool PlaneSphereIntersection(const SPlane& plane, const SphereHull& sphere)
	{
		float spCenterToNormalProjection = sphere.getPosition().Dot(plane.Normal());
		float spherePlaneDist = spCenterToNormalProjection + plane.D();
		return (spherePlaneDist < sphere.r);
	}


	//Returns true if the sphere's situated, even partially, in the positive direction of plane's normal from the plane
	static bool SphereInsidePlane(const SPlane& plane, const SphereHull& sphere)
	{
		float spCenterToNormalProjection = sphere.getPosition().Dot(plane.Normal());
		float spherePlaneDist = spCenterToNormalProjection + plane.D();
		return (spherePlaneDist + sphere.r > 0);
	}



	static bool FrustumSphereIntersection(const Frustum& frustum, const SphereHull& sphere)
	{
		for (int i = 0; i < 6; ++i)
		{
			if (!SphereInsidePlane(frustum._planes[i], sphere))
				return false;
		}
		return true;
	}



	static bool ConeInsidePlane(const SPlane& plane, const Cone& cone)
	{
		float coneTipOnPlaneProjection = cone._tip.Dot(plane.Normal());
		float coneTipPlaneDist = coneTipOnPlaneProjection + plane.w;	//I would use .D() but it's not even inline smh
		
		if (coneTipPlaneDist < 0.f)
			return false;

		SVec3 m = (plane.Normal().Cross(cone._dir)).Cross(cone._dir);	//"downwards" vector along the cone's rim surface
		SVec3 Q = cone._tip + cone._d * cone._dir + cone._radius * m;		//closest point on the cone's rim towards the plane

		return (Q.Dot(plane.Normal()) + plane.w > 0.f);
	}



	static bool FrustumConeIntersection(const Frustum& frustum, const Cone& cone)
	{
		for (int i = 0; i < 6; ++i)
		{
			if (!ConeInsidePlane(frustum._planes[i], cone))
				return false;
		}
		return true;
	}

}
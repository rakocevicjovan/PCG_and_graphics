#pragma once
#include <vector>
#include <map>
#include <set>
#include <list>
#include <d3d11.h>
#include "Controller.h"

class Actor;
class Model;
class Mesh;


struct HitResult
{
	bool hit = false;
	SVec3 resolutionVector = SVec3();
	float sqPenetrationDepth = 0.f;

	HitResult() {}
	HitResult(bool h, SVec3 rv, float sqpd) : hit(h), resolutionVector(rv), sqPenetrationDepth(sqpd) {}
};



enum BoundingVolumeType
{
	BVT_AABB,
	BVT_SPHERE
};



struct Hull
{
	virtual HitResult intersect(const Hull* other, BoundingVolumeType otherType) const = 0;
	virtual SVec3 getPosition() const = 0;
	virtual void setPosition(SVec3 newPos) = 0;
};



struct AABB : Hull
{
	SVec3 minPoint, maxPoint;

	virtual HitResult intersect(const Hull* other, BoundingVolumeType otherType) const override;
	virtual SVec3 getPosition() const
	{ 
		return SVec3(minPoint.x + maxPoint.x, minPoint.y + maxPoint.y, minPoint.z + maxPoint.z) * 0.5f; 
	}
	virtual void setPosition(SVec3 newPos)
	{
		SVec3 posDelta = newPos - getPosition();
		minPoint += posDelta;
		maxPoint += posDelta;
	}

	bool operator ==(AABB other) { return ( (minPoint - other.minPoint + maxPoint - other.maxPoint).LengthSquared() > 0.001f ); }

	std::vector<SVec3> getVertices() const;

	std::vector<SPlane> getPlanes() const;
};



struct SphereHull : Hull
{
	SVec3 c;
	float r;

	virtual HitResult intersect(const Hull* other, BoundingVolumeType otherType) const override;
	virtual SVec3 getPosition() const { return c; }
	virtual void setPosition(SVec3 newPos) { c = newPos; }
};



struct Collider
{
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



struct CellKey
{
	int x = 0, y = 0, z = 0;
	
	CellKey(int ix, int iy, int iz) : x(ix), y(iy), z(iz) {};
	
	CellKey() : x(-1), y(-1), z(-1) {};

#define FASTFLOOR(x) ( (x >= 0.f) ? ((int)x) : ((int)x-1 ) )

	CellKey(const SVec3& pos, float invCellSize)
	{
		x = FASTFLOOR(pos.x * invCellSize);
		y = FASTFLOOR(pos.y * invCellSize);
		z = FASTFLOOR(pos.z * invCellSize);
	}
#undef FASTFLOOR
	
	inline void assign(const SVec3& in) { x = floor(in.x); y = floor(in.y); z = floor(in.z); }

	int total() const { return x * y * z + y * z + z; }

	bool operator <(const CellKey& other) const { return total() < other.total(); }
};



struct GridCell
{
	std::set<Hull*> hulls;
};



struct Grid
{
	Grid() {};
	Grid(float cellsize) : _cellsize(cellsize) {}
	float _cellsize = 32.f;
	float invCellSize = 1.f / _cellsize;
	std::map<CellKey, GridCell> cells;

	void addAABB(AABB* h);
	void addSphere(SphereHull* h);
	//void removeAABB(Hull *h);
	//void removeSphere(Hull* h);
};




class CollisionEngine
{
	Grid grid;

	Controller* _controller;
	std::list<Collider*> _colliders;

public:

	Hull* genSphereHull(Mesh* mesh);
	Hull* genBoxHull(Mesh* mesh);

	CollisionEngine();
	~CollisionEngine();

	void init() {}

	void registerModel(Model& model, BoundingVolumeType bvt);
	void registerActor(Actor& actor, BoundingVolumeType bvt);
	void unregisterModel(Model& model);
	void unregisterActor(Actor& actor);
	void addToGrid(Collider* collider);
	void removeFromGrid(Collider& collider);
	void update();

	void registerController(Controller& controller);
	SVec3 resolvePlayerCollision(const SMatrix& playerTransform, SVec3& velocity);
};
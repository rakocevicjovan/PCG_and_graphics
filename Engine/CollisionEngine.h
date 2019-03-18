#pragma once
#include <vector>
#include <map>
#include "Controller.h"

class Model;
class Mesh;

enum BoundingVolumeType
{
	BVT_AABB,
	BVT_SPHERE
};



struct Hull
{
	virtual bool intersect(const Hull* other, BoundingVolumeType otherType) const = 0;
	virtual SVec3 getPosition() const = 0;
};



struct AABB : Hull
{
	SVec3 min, max;

	virtual bool intersect(const Hull* other, BoundingVolumeType otherType) const override;
	virtual SVec3 getPosition() const { return (min + max) * 0.5f; }

	bool operator ==(AABB other) { return ( (min - other.min + max - other.max).LengthSquared() > 0.001f ); }

	std::vector<SVec3> getAllVertices()
	{
		return 
		{
			min,	//lower half
			SVec3(min.x, min.y, max.z),
			SVec3(max.x, min.y, min.z),
			SVec3(max.x, min.y, max.z),
			max,	//higher half
			SVec3(min.x, max.y, max.z),
			SVec3(max.x, max.y, min.z),
			SVec3(min.x, max.y, min.z),
		};
	}
};



struct SphereHull : Hull
{
	SVec3 c;
	float r;

	virtual bool intersect(const Hull* other, BoundingVolumeType otherType) const override;
	virtual SVec3 getPosition() const { return c; }
};



struct Collider
{
	Collider(BoundingVolumeType type, Model* m, std::vector<Hull*> hullptrs) : BVT(type), parent(m), hulls(hullptrs) {}

	~Collider() { }

	void ReleaseMemory() { for (auto* hull : hulls) delete hull; }

	bool operator ==(const Collider& other) const { return parent == other.parent; }

	BoundingVolumeType BVT;
	Model* parent;
	std::vector<Hull*> hulls;
	bool dynamic;

	static bool AABBSphereIntersection(const AABB& b, const SphereHull& s);
	static bool SphereSphereIntersection(const SphereHull& s1, const SphereHull& s2);
	static bool AABBAABBIntersection(const AABB& a, const AABB& b);
	static bool RaySphereIntersection(const SRay& ray, const SphereHull& s);
	static bool RayAABBIntersection(const SRay& ray, const AABB& b);
	static float SQD_PointAABB(SVec3 p, AABB b);

	static bool RayPlaneIntersection(const SRay& ray, const SVec3& a, const SVec3& b, const SVec3& c, SVec3& intersectionPoint);
	static bool RayTriangleIntersection(const SRay& ray, const SVec3& a, const SVec3& b, const SVec3& c);

	bool Collider::Collide(const Collider& other, SVec3& resolutionVector);
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


	int total() const 
	{ 
		return x * y * z + y * z + z; 
	}

	bool operator <(const CellKey& other) const { return total() < other.total(); }
};



struct GridCell
{
	std::vector<Hull*> hulls;
};



struct Grid
{
	Grid() {};
	Grid(float cellsize) : _cellsize(cellsize) {}
	float _cellsize = 64.f;
	float invCellSize = 1.f / _cellsize;
	std::map<CellKey, GridCell> cells;

	void addAABB(AABB* h);
	void addSphere(SphereHull* h);
};

class CollisionEngine
{
	Controller* _controller;
	std::vector<Model*> _models;
	std::vector<Collider> _colliders;

	Grid grid;

	Hull* genSphereHull(Mesh* mesh);
	Hull* genBoxHull(Mesh* mesh);
	Collider generateCollider(Model* model, BoundingVolumeType bvt);
	
public:
	CollisionEngine();
	~CollisionEngine();

	void registerModel(Model* model, BoundingVolumeType bvt);
	void unregisterModel(const Model* model);
	void addToGrid(const Collider& collider);

	void registerController(Controller& controller);
	SVec3 resolvePlayerCollision(const SMatrix& playerTransform);
};
#pragma once
#include <vector>
#include <map>
#include "Controller.h"
#include "Terrain.h"

class Model;
class Mesh;

enum BoundingVolumeType
{
	BVT_AABB,
	BVT_SPHERE
};



struct Hull
{
	virtual bool intersect(Hull* other, BoundingVolumeType otherType) = 0;
	virtual SVec3 getPosition() = 0;
};



struct AABB : Hull
{
	SVec3 min, max;

	virtual bool intersect(Hull* other, BoundingVolumeType otherType) override;
	virtual SVec3 getPosition() { return (min + max) * 0.5f; }
};



struct SphereHull : Hull
{
	SVec3 c;
	float r;

	virtual bool intersect(Hull* other, BoundingVolumeType otherType) override;
	virtual SVec3 getPosition() { return c; }
};



struct Collider
{
	Collider(BoundingVolumeType type, Model* m, std::vector<Hull*> hullptrs) : BVT(type), parent(m), hulls(hullptrs) {}

	~Collider() { for (auto* hull : hulls) delete hull; }

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
	inline void assign(const SVec3& in) { x = floor(in.x); y = floor(in.y); z = floor(in.z); }

	int total() const 
	{ 
		return x * y * z + y * z + z; 
	}

	bool operator <(const CellKey& other) const { return total() < other.total(); }
};



struct GridCell
{
	UINT x, y, z;
	std::vector<Procedural::Terrain*> terrains;

	GridCell() {};
	GridCell(CellKey ck) : x(ck.x), y(ck.y), z(ck.z) {};
};



struct Grid
{
	//UINT _w, _h, _d;	Grid(UINT w, UINT h, UINT d) : _w(w), _h(h), _d(d)	{ cells.reserve(_w * _h * _d); }
	const float CELLSIZE = 33.f;
	float invCellSize = 1.f / CELLSIZE;
	std::map<CellKey, GridCell> cells;

	void populateCells(std::vector<Procedural::Terrain*>& terrain);
};

class CollisionEngine
{
	Controller* _controller;
	std::vector<Model*> _models;
	std::vector<Collider> _colliders;

	Hull* genSphereHull(Mesh* mesh);
	Hull* genBoxHull(Mesh* mesh);
	Collider generateCollider(Model* model, BoundingVolumeType bvt);
	
public:
	CollisionEngine();
	~CollisionEngine();

	void registerModel(Model* model, BoundingVolumeType bvt);
	void unregisterModel(const Model* model);

	void registerController(Controller* controller);
	void notifyController(const SVec3& resolution) const;

	SVec3 adjustHeight(const SVec3& playerPos);

	Grid grid;
};


#pragma once
#include <vector>
#include <map>
#include <d3d11.h>
#include "Controller.h"
#include "Model.h"


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
};



struct AABB : Hull
{
	SVec3 min, max;

	virtual HitResult intersect(const Hull* other, BoundingVolumeType otherType) const override;
	virtual SVec3 getPosition() const { return (min + max) * 0.5f; }

	bool operator ==(AABB other) { return ( (min - other.min + max - other.max).LengthSquared() > 0.001f ); }

	std::vector<SVec3> getVertices() const;

	std::vector<SPlane> getPlanes() const;
};



struct SphereHull : Hull
{
	SVec3 c;
	float r;

	virtual HitResult intersect(const Hull* other, BoundingVolumeType otherType) const override;
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
	float _cellsize = 32.f;
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

	ID3D11Device* _device;
	ID3D11DeviceContext* _deviceContext;

	Grid grid;

	Hull* genSphereHull(Mesh* mesh);
	Hull* genBoxHull(Mesh* mesh);
	Collider generateCollider(Model* model, BoundingVolumeType bvt);
	
public:

	std::vector<Model> _colModels;

	CollisionEngine();
	~CollisionEngine();

	void init(ID3D11Device* d, ID3D11DeviceContext* dc) { _device = d; _deviceContext = dc; }

	void registerModel(Model* model, BoundingVolumeType bvt);
	void unregisterModel(const Model* model);
	void addToGrid(const Collider& collider);

	void registerController(Controller& controller);
	SVec3 resolvePlayerCollision(const SMatrix& playerTransform, SVec3& velocity);
};
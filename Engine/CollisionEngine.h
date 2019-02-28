#pragma once
#include <vector>
#include "Model.h"
#include "Controller.h"



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

	bool Collider::Collide(const Collider& other, SVec3& resolutionVector);
};



class CollisionEngine
{

	Controller* _controller;
	std::vector<Model*> _models;

	Hull* genSphereHull(Mesh* mesh);
	Hull* genBoxHull(Mesh* mesh);
	
public:
	CollisionEngine();
	~CollisionEngine();

	void registerModel(Model* model, BoundingVolumeType bvt);
	Collider generateHulls(Model* model, BoundingVolumeType bvt);
	void unregisterModel(const Model* model);

	void registerController(Controller* controller);
	void notifyController(const SVec3& resolution) const;
};


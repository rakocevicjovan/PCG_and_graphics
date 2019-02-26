#pragma once
#include <vector>
#include "Model.h"
#include "Controller.h"
//#include "QuickHull.hpp"


enum BoundingVolumeType
{
	BVT_AABB,
	BVT_SPHERE
	//BVT_QUICKHULL
};



struct Hull
{
	virtual bool intersect(Hull* other) = 0;
};

struct AABB : Hull
{
	SVec3 min, max;

	virtual bool intersect(Hull* other) override
	{

	}
};

struct SphereHull : Hull
{
	SVec3 c;
	float r;

	virtual bool intersect(Hull* other) override
	{

	}
};


//struct CHull : Hull	{ quickhull::ConvexHull<float> convexHull; };

struct Collider
{
	Collider(BoundingVolumeType type, Model* m, std::vector<Hull*> hullptrs) : BVT(type), parent(m), hulls(hullptrs) {}

	~Collider() { for (auto* hull : hulls) delete hull; }

	BoundingVolumeType BVT;
	Model* parent;
	std::vector<Hull*> hulls;

	bool Collider::Collide(const Collider& other)
	{

		bool collides = false;

		for (auto hull : hulls)
		{
			switch (BVT)
			{
			case BVT_AABB:
				return AABBintersect();
				break;
			case BVT_SPHERE:

				break;
			}
		}
	
	}
};



class CollisionEngine
{

	Controller* _controller;
	std::vector<Model*> _models;

	Hull* genSphereHull(Mesh* mesh);
	Hull* genBoxHull(Mesh* mesh);
	//Hull* genQuickHull(Mesh* mesh);
	
public:
	CollisionEngine();
	~CollisionEngine();

	void registerModel(Model* model, BoundingVolumeType bvt);
	Collider generateHulls(Model* model, BoundingVolumeType bvt);
	void unregisterModel(const Model* model);

	void registerController(Controller* controller);
	void notifyController(const SVec3& resolution) const;
};


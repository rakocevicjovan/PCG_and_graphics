#pragma once
#include "Hull.h"
#include "ColFuncs.h"
#include "GameObject.h"


class Collider
{
public:
	BoundingVolumeType BVT;
	Actor* parent;
	std::vector<Hull*> hulls;	//this could be templated... unless i want to support multiple, yet different type, hulls
	bool dynamic;


	Collider() {}
	Collider(BoundingVolumeType type, Actor* a, std::vector<Hull*> hullptrs)
		: BVT(type), parent(a), hulls(hullptrs) {}

	Collider(BoundingVolumeType type, Actor* a, bool dyn)
		: BVT(type), parent(a), dynamic(dyn) {}

	~Collider() { }



	void ReleaseMemory()
	{
		for (Hull* hull : hulls)
		{
			delete hull;
			hull = nullptr;
		}
	}


	//each collider has only one parent so this works I guess... could it simply work by default though???
	bool operator ==(const Collider& other) const
	{
		return parent == other.parent;
	}



	void updateHullPositions()
	{
		for (auto& hull : hulls)
			hull->setPosition(parent->getPosition() + hull->getPosition());
	}



	HitResult Collide(const Collider& other, SVec3& resolutionVector)
	{
		HitResult hitRes;

		for (Hull* hull1 : hulls)
		{
			for (Hull* hull2 : other.hulls)
			{
				if (BVT == BVT_AABB)
					hitRes = reinterpret_cast<AABB*>(hull1)->intersect(hull2, other.BVT);
				if (BVT == BVT_SPHERE)
					hitRes = reinterpret_cast<SphereHull*>(hull1)->intersect(hull2, other.BVT);

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
};
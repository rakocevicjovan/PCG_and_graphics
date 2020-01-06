#pragma once
#include "Hull.h"
#include "ColFuncs.h"

class Actor;

class Collider
{
protected:
	std::vector<Hull*> _hulls;	//this could be templated... unless I need various hulls within a collider (it's likely)

public:
	BoundingVolumeType BVT;
	Actor* parent;
	
	bool dynamic;

	Collider() {}

	Collider(BoundingVolumeType type, Actor* a, std::vector<Hull*> hullptrs)
		: BVT(type), parent(a), _hulls(hullptrs) {}

	Collider(BoundingVolumeType type, Actor* a, bool dyn)
		: BVT(type), parent(a), dynamic(dyn) {}

	~Collider() {}

	void addHull(Hull* hull)
	{
		_hulls.push_back(hull);
		_hulls.back()->_collider = this;
	}

	void clearHulls()
	{
		for (Hull* hull : _hulls)
		{
			delete hull;
			hull = nullptr;
		}

		_hulls.clear();
	}

	Hull* getHull(UINT i)
	{
		return _hulls[i];
	}

	const std::vector<Hull*>& getHulls() const
	{
		return _hulls;
	}

	void ReleaseMemory();
	bool operator==(const Collider& other) const;
	void updateHullPositions();
	HitResult Collide(const Collider& other, SVec3& resolutionVector);
};
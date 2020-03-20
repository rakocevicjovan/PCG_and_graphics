#pragma once
#include "Hull.h"
#include "ColFuncs.h"

class Actor;

class Collider
{
protected:
	//this could be templated... unless I need various hulls within a collider (it's likely)
	std::vector<Hull*> _hulls;	

	void releaseMemory();

public:
	BoundingVolumeType BVT;
	Actor* _parent;
	
	bool dynamic;

	Collider() {}

	Collider(Actor* a) : _parent(a) {}

	Collider(BoundingVolumeType type, Actor* a, std::vector<Hull*> hullptrs)
		: BVT(type), _parent(a), _hulls(hullptrs) {}

	Collider(BoundingVolumeType type, Actor* a, bool dyn)
		: BVT(type), _parent(a), dynamic(dyn) {}

	~Collider()
	{
		releaseMemory();
	}


	inline void addHull(Hull* hull)
	{
		_hulls.push_back(hull);
		_hulls.back()->_collider = this;
	}

	bool operator==(const Collider& other) const;
	HitResult Collide(const Collider& other, SVec3& resolutionVector) const;

	void updateHullPositions();
	void deleteAndClearHulls();

	inline void clearHulls() { _hulls.clear(); }

	inline Hull* getHull(UINT i) const { return _hulls[i]; }

	inline std::vector<Hull*>& getHulls() { return _hulls; };

	inline const std::vector<Hull*>& getHulls() const { return _hulls; }
};
#pragma once
#include "Hull.h"
#include "ColFuncs.h"

class Actor;

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

	~Collider() {}



	void ReleaseMemory();
	bool operator==(const Collider& other) const;

	void updateHullPositions();
	HitResult Collide(const Collider& other, SVec3& resolutionVector);
};
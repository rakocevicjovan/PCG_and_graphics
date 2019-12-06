#pragma once
#include "Hull.h"

class Actor;
class Model;
class Mesh;


class Collider
{
public:
	BoundingVolumeType BVT;
	Model* modParent;		//delete this eventually... it should all be actors, static or dynamic however, determined by collider type
	Actor* actParent;
	std::vector<Hull*> hulls;	//this could be templated... unless i want to support multiple, yet different type, hulls
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


	//each collider has only one parent so this works I guess... could it simply work by default though???
	bool operator ==(const Collider& other) const
	{
		if (dynamic == true)
			return actParent == other.actParent;
		else
			return modParent == other.modParent;
	}

	HitResult Collider::Collide(const Collider& other, SVec3& resolutionVector);
};
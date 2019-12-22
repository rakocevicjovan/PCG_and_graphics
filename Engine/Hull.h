#pragma once
#include <vector>
#include "Math.h"
#include "HitResult.h"

class Collider;

enum BoundingVolumeType
{
	BVT_AABB,
	BVT_SPHERE
};



class Hull
{
public:
	Collider* _collider;	//points to the collider, but does not own it

	virtual HitResult intersect(const Hull* other, BoundingVolumeType otherType) const = 0;
	inline virtual SVec3 getPosition() const = 0;
	inline virtual void setPosition(const SVec3& newPos) = 0;
};



class AABB : public Hull
{
public:
	SVec3 minPoint, maxPoint;

	AABB() {}


	AABB(SVec3 center, SVec3 halfSize)
	{
		minPoint = center - halfSize;
		maxPoint = center + halfSize;
	}


	HitResult intersect(const Hull* other, BoundingVolumeType otherType) const override;


	inline SVec3 getPosition() const override
	{
		return SVec3(minPoint + maxPoint) * 0.5f;
	}


	void setPosition(const SVec3& newPos) override
	{
		SVec3 posDelta = newPos - getPosition();
		minPoint += posDelta;
		maxPoint += posDelta;
	}


	bool operator ==(AABB other)	//is this really required??? Forgot why I wrote it... should be discernible by pointers...
	{ 
		return ( ((minPoint - other.minPoint) + (maxPoint - other.maxPoint)).LengthSquared() < 0.001f );
	}

	std::vector<SVec3> getVertices() const;

	std::vector<SPlane> getPlanes() const;

	inline SVec3 getHalfSize() const { return ((maxPoint - minPoint) * 0.5f); }
	inline SVec3 getSize() const { return maxPoint - minPoint; }
};



class SphereHull : public Hull
{
public:
	SVec3 ctr;
	float r;

	SphereHull() {};
	SphereHull(const SVec3& pos, float rad) : ctr(pos), r(rad) {};

	HitResult intersect(const Hull* other, BoundingVolumeType otherType) const override;
	inline SVec3 getPosition() const override { return ctr; }
	inline void setPosition(const SVec3& newPos) override { ctr = newPos; }
};
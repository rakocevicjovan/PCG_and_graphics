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
	Collider* c;

	virtual HitResult intersect(const Hull* other, BoundingVolumeType otherType) const = 0;
	virtual SVec3 getPosition() const = 0;
	virtual void setPosition(SVec3 newPos) = 0;
};



class AABB : public Hull
{
public:
	SVec3 minPoint, maxPoint;

	virtual HitResult intersect(const Hull* other, BoundingVolumeType otherType) const override;

	virtual SVec3 getPosition() const
	{
		return SVec3(minPoint.x + maxPoint.x, minPoint.y + maxPoint.y, minPoint.z + maxPoint.z) * 0.5f;
	}

	virtual void setPosition(SVec3 newPos) override
	{
		SVec3 posDelta = newPos - getPosition();
		minPoint += posDelta;
		maxPoint += posDelta;
	}

	bool operator ==(AABB other)
	{ 
		return ((minPoint - other.minPoint + maxPoint - other.maxPoint).LengthSquared() > 0.001f);
	}

	std::vector<SVec3> getVertices() const;

	std::vector<SPlane> getPlanes() const;
};



class SphereHull : public Hull
{
public:
	SVec3 c;
	float r;

	virtual HitResult intersect(const Hull* other, BoundingVolumeType otherType) const override;
	virtual SVec3 getPosition() const { return c; }
	virtual void setPosition(SVec3 newPos) { c = newPos; }
};
#pragma once
#include <vector>
#include "Math.h"
#include "HitResult.h"
#include "Cone.h"

class Collider;

enum BoundingVolumeType : uint8_t
{
	BVT_AABB,
	BVT_SPHERE
};



class Hull
{
public:
	Collider* _collider{};	//points to the collider, but does not own it

	virtual HitResult intersect(const Hull* other, BoundingVolumeType otherType) const = 0;
	inline virtual SVec3 getPosition() const = 0;
	inline virtual float getExtent() const = 0;
	inline virtual void setPosition(const SVec3& newPos) = 0;
};



class AABB final : public Hull
{
public:
	SVec3 minPoint, maxPoint;

	AABB() {}


	AABB::AABB(SVec3 center, SVec3 halfSize)
	{
		minPoint = center - halfSize;
		maxPoint = center + halfSize;
	}


	static AABB fromMinMax (const SVec3& min, const SVec3& max)
	{
		return { min, max };
	}


	HitResult AABB::intersect(const Hull* other, BoundingVolumeType otherType) const;

	void setPosition(const SVec3& newPos) override;
	bool operator ==(AABB other)	//is this really required??? Forgot why I wrote it... should be discernible by pointers...
	{
		return (((minPoint - other.minPoint) + (maxPoint - other.maxPoint)).LengthSquared() < 0.001f);
	}

	std::vector<SVec3> getVertices() const;

	std::vector<SPlane> getPlanes() const;

	inline SVec3 getHalfSize() const { return ((maxPoint - minPoint) * 0.5f); }
	inline SVec3 getSize() const { return maxPoint - minPoint; }

	inline SVec3 getPosition()	const override { return SVec3(minPoint + maxPoint) * 0.5f; }
	inline float getExtent()	const override { return getHalfSize().Length(); }
};



class SphereHull final : public Hull
{
public:
	SVec3 ctr{};
	float r{};

	SphereHull() {};
	SphereHull(const SVec3& pos, float rad) : ctr(pos), r(rad) {};
	SphereHull(const SVec4& posRadius) : ctr(posRadius.x, posRadius.y, posRadius.z), r(posRadius.w) {}

	HitResult intersect(const Hull* other, BoundingVolumeType otherType) const override;
	inline SVec3 getPosition()	const override { return ctr; }
	inline float getExtent()	const override { return r; }
	inline void setPosition(const SVec3& newPos) override { ctr = newPos; }
};
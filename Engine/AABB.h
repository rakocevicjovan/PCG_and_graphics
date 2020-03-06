/*

#pragma once
#include "Hull.h"


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
*/
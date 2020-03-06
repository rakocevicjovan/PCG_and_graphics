/*
#pragma once
#include "Hull.h"


class SphereHull : public Hull
{
public:
	SVec3 ctr;
	float r;

	SphereHull() {};
	SphereHull(const SVec3& pos, float rad) : ctr(pos), r(rad) {};
	SphereHull(const SVec4& posRadius) : ctr(posRadius.x, posRadius.y, posRadius.z), r(posRadius.w) {}

	HitResult intersect(const Hull* other, BoundingVolumeType otherType) const override;
	inline SVec3 getPosition()	const override { return ctr; }
	inline float getExtent()	const override { return r; }
	inline void setPosition(const SVec3& newPos) override { ctr = newPos; }
};
*/
#pragma once
#include "Bone.h"
#include "Math.h"


class Skeleton
{
public:
	SMatrix _globalInverseTransform;
	std::vector<Bone> _bones;
	uint32_t _numInfluenceBones{ 0u };

	// Slow, but not used at game runtime and really convenient
	int getBoneIndex(const char* name) const;
	int getInfluenceBoneIndex(const char* name) const;

	inline uint32_t getBoneCount() const { return static_cast<uint32_t>(_bones.size()); }

	template <typename Archive> 
	void serialize(Archive& ar)
	{
		ar(_globalInverseTransform, _bones, _numInfluenceBones);
	}
};
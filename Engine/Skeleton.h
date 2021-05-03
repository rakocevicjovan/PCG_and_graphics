#pragma once
#include "Bone.h"
#include "Math.h"


class Skeleton
{
public:
	SMatrix _globalInverseTransform;
	std::vector<Bone> _bones;

	// Slow, but not used at game runtime and really convenient
	int getBoneIndex(const char* name) const;

	inline UINT getBoneCount() { return _bones.size(); }

	template <typename Archive> 
	void serialize(Archive& ar) const
	{
		ar(_globalInverseTransform, _bones);
	}
};
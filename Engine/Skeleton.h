#pragma once
#include <string>
#include "Bone.h"
#include "Math.h"

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>



class Skeleton
{
public:
	SMatrix _globalInverseTransform;
	std::vector<Bone> _bones;

	// Slow, but not used at runtime and really convenient
	int getBoneIndex(const std::string& name);

	inline UINT getBoneCount() { return _bones.size(); }

	template <typename Archive> 
	void serialize(Archive& ar) const
	{
		ar(_globalInverseTransform, _bones);
	}
};
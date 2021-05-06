#pragma once
#include "Math.h"
#include <string>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/utility.hpp>

using BoneIndex = uint16_t;

class Bone
{
public:

	static constexpr BoneIndex INVALID_INDEX{ static_cast<BoneIndex>(~0) };

public:

	std::string _name;	// Big, but SSO is worth it... maybe? Bones should be a SoA anyways probably

	SMatrix _invBindPose;
	SMatrix _localMatrix;

	BoneIndex _parent{0u};
	std::pair<BoneIndex, uint16_t> _children { INVALID_INDEX, INVALID_INDEX };	// Span
	bool _isInfluenceBone{ false };

	Bone() = default;

	Bone(const char* name, SMatrix offset, BoneIndex parentIdx = Bone::INVALID_INDEX)
		:_name(name), _invBindPose(offset), _parent(parentIdx)
	{}

	inline bool isInfluenceBone() const
	{
		return _isInfluenceBone;
	}

	inline const char* name() const
	{
		return _name.c_str();
	}

	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(_name, _isInfluenceBone, _invBindPose, _localMatrix, _parent, _children);
	}
};
#pragma once
#include "Math.h"
#include <string>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>

using BoneIndex = uint16_t;

class Bone
{
public:

	static constexpr BoneIndex INVALID_INDEX{ static_cast<BoneIndex>(~0) };
	std::string _name;

	SMatrix _invBindPose;
	SMatrix _localMatrix;

	BoneIndex _parent{0u};
	std::pair<BoneIndex, uint16_t> _children { INVALID_INDEX, INVALID_INDEX };	// Span

	Bone() = default;

	Bone(const char* name, SMatrix offset, BoneIndex parentIdx = Bone::INVALID_INDEX)
		:_name(name), _invBindPose(offset), _parent(parentIdx) {}


	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(_name, _invBindPose, _localMatrix, _parent);
	}
};
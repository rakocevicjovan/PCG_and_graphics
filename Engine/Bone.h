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

	int _index{~0};
	std::string _name;

	SMatrix _invBindPose;
	SMatrix _localMatrix;

	BoneIndex _parent{0u};
	std::vector<BoneIndex> _children;

	Bone() = default;

	Bone(int index, const char* name, SMatrix offset)
		: _index(index), _name(name), _invBindPose(offset) {}


	template <typename Archive>
	void serialize(Archive& ar)
	{
		ar(_index, _name, _invBindPose, _localMatrix, _parent, _children);
	}
};
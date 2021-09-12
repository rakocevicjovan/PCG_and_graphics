#pragma once

#include <string>

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
	std::pair<BoneIndex, uint16_t> _children { INVALID_INDEX, 0ui16 };	// Span
	bool _isInfluenceBone{ false };

	Bone() = default;

	Bone(const char* name, SMatrix localMatrix, BoneIndex parentIdx = Bone::INVALID_INDEX)
		:_name(name), _localMatrix(localMatrix), _parent(parentIdx)
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
#pragma once
#include "Math.h"
#include <vector>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>


struct PosFrame
{
	SVec3 pos;
	float tick;

	PosFrame() {}
	PosFrame(float tick, SVec3 pos) : pos(pos), tick(tick) {}

	template<class Archive> void serialize(Archive& ar) { ar(pos, tick); }
};


struct RotFrame
{
	SQuat rot{};
	float tick{};

	RotFrame() noexcept {}
	RotFrame(float tick, SQuat rot) noexcept : rot(rot), tick(tick) {}

	template<class Archive> void serialize(Archive& ar) { ar(rot, tick); }
};


struct SclFrame
{
	SVec3 scale{};
	float tick{};

	SclFrame() noexcept {}
	SclFrame(float tick, SVec3 scale) noexcept : scale(scale), tick(tick) {}

	template<class Archive> void serialize(Archive& ar) { ar(scale, tick); }
};


struct AnimChannel
{
	std::string _boneName;

	std::vector<SclFrame> _sKeys;
	std::vector<RotFrame> _rKeys;
	std::vector<PosFrame> _pKeys;

	AnimChannel() noexcept {}

	AnimChannel(int p, int r, int s) noexcept;

	SMatrix getInterpolatedTransform(float currentTick, float t) const;

	template<class Archive>
	void serialize(Archive& ar)
	{
		ar(_boneName, _rKeys, _pKeys, _sKeys);
	}
};
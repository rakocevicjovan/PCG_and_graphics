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
	SQuat rot;
	float tick;

	RotFrame() {}
	RotFrame(float tick, SQuat rot) : rot(rot), tick(tick) {}

	template<class Archive> void serialize(Archive& ar) { ar(rot, tick); }
};


struct SclFrame
{
	SVec3 scale;
	float tick;

	SclFrame() {}
	SclFrame(float tick, SVec3 scale) : scale(scale), tick(tick) {}

	template<class Archive> void serialize(Archive& ar) { ar(scale, tick); }
};


struct AnimChannel
{
	std::string _boneName;

	std::vector<SclFrame> _sKeys;
	std::vector<RotFrame> _rKeys;
	std::vector<PosFrame> _pKeys;

	AnimChannel() {}

	AnimChannel(int p, int r, int s);

	SMatrix getInterpolatedTransform(float currentTick, float t) const;

	template<class Archive>
	void serialize(Archive& ar)
	{
		ar(_boneName, _rKeys, _pKeys, _sKeys);
	}
};
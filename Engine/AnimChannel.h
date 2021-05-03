#pragma once
#include "Math.h"
#include <vector>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>


template <typename FrameData>
struct KeyFrame
{
	FrameData frameData{};
	float tick{ 0.f };

	KeyFrame() = default;

	KeyFrame(FrameData frameData, float tick)
		: frameData(frameData), tick(tick)
	{}

	template<class Archive>
	void serialize(Archive& ar)
	{ 
		ar(frameData, tick);
	}
};


struct AnimChannel
{
	std::string _boneName;

	std::vector<KeyFrame<SVec3>> _sKeys;
	std::vector<KeyFrame<SQuat>> _rKeys;
	std::vector<KeyFrame<SVec3>> _pKeys;

	AnimChannel() noexcept {}

	AnimChannel(int p, int r, int s) noexcept;

	SMatrix getInterpolatedTransform(float currentTick, float t) const;

	template<class Archive>
	void serialize(Archive& ar)
	{
		ar(_boneName, _rKeys, _pKeys, _sKeys);
	}
};
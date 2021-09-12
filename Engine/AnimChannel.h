#pragma once

#include "KeyFrame.h"


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
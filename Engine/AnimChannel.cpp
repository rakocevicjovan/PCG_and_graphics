#include "pch.h"
#include "AnimChannel.h"


AnimChannel::AnimChannel(int p, int r, int s) noexcept
{
	_pKeys.reserve(p);
	_rKeys.reserve(r);
	_sKeys.reserve(s);
}


namespace
{
	template <typename AnimationData, typename AnimationFrame>
	inline static AnimationData GetFrameDataAtTime(const std::vector<AnimationFrame>& keyFrames, float currentTick, float t)
	{
		auto numKeys = keyFrames.size();
		if (numKeys == 1u) [[unlikely]]
		{
			return keyFrames[0].frameData;
		}
		else
		{
			auto it = std::upper_bound(keyFrames.begin(), keyFrames.end(), currentTick,
				[](float currentTick, AnimationFrame frame) -> bool
				{
					return currentTick < frame.tick;
				});

			if constexpr(std::is_same_v<AnimationData, SQuat>)
			{
				return SQuat::Slerp((it - 1)->frameData, it->frameData, t);
			}
			else
			{
				return Math::lerp((it - 1)->frameData, it->frameData, t);
			}
		}
	}
}


SMatrix AnimChannel::getInterpolatedTransform(float currentTick, float t) const
{
	SVec3 pos = GetFrameDataAtTime<SVec3>(_pKeys, currentTick, t);
	SVec3 scale = GetFrameDataAtTime<SVec3>(_sKeys, currentTick, t);
	SQuat quat = GetFrameDataAtTime<SQuat>(_rKeys, currentTick, t);

	return DirectX::XMMatrixAffineTransformation(scale, SVec3(), quat, pos);
}
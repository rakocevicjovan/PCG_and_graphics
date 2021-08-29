#include "pch.h"

#include "RuntimeAnimation.h"
#include "Animation.h"
#include "Skeleton.h"
#include "KeyFrame.h"
#include "Math.h"


namespace
{
	void MergeChannels(Animation& animation)
	{
		auto& channels = *animation.getChannels();
		
		for (auto& channel : channels)
		{

		}
	}
}

// Rough prototype, basically put all KeyFrames of the same type together to start off. Sort by time. 
// Might be better if they are all in one vector but there are issues with that
RuntimeAnimation BuildRuntimeAnimation(const Skeleton& skeleton, const Animation& animation)
{
	// For now, stores these types. Later on, it will store compressed version
	std::vector<KeyFrame<SVec3>> positions;	// 12 bytes -> 6 bytes
	std::vector<KeyFrame<SQuat>> rotations;	// 16 bytes	-> 4 bytes (possibly, could use 6 or 8)
	std::vector<KeyFrame<SVec3>> scales;	// 12 bytes -> 4 bytes (very likely enough)

	// For now just reserve a ton, assume 50 bones with 40 KeyFrames or something like that
	positions.reserve(2000);
	rotations.reserve(2000);
	scales.reserve(2000);

	const auto& animChannels = *animation.getChannels();

	// Skeletons are stored breadth first right now
	for (const auto& bone : skeleton._bones)
	{
		const auto* channel = animation.getAnimChannel(animation.getAnimChannelIndex(bone.name()));

		// Not every bone has a channel
		if (!channel)
			continue;

		for (auto& posKeyFrame : channel->_pKeys)
		{
			positions.push_back(posKeyFrame);
		}

		for (auto& rotKeyFrame : channel->_rKeys)
		{
			rotations.push_back(rotKeyFrame);
			//assert(abs(rotKeyFrame.tick - round(rotKeyFrame.tick)) < 0.001);
		}

		for (auto& sclKeyFrame : channel->_sKeys)
		{
			scales.push_back(sclKeyFrame);
		}
	}

	std::stable_sort(positions.begin(), positions.end(), 
		[](const KeyFrame<SVec3>& left, const KeyFrame<SVec3>& other)
		{
			return left.tick < other.tick;
		});

	std::stable_sort(rotations.begin(), rotations.end(),
		[](const KeyFrame<SQuat>& left, const KeyFrame<SQuat>& other)
		{
			return left.tick < other.tick;
		});

	std::stable_sort(scales.begin(), scales.end(),
		[](const KeyFrame<SVec3>& left, const KeyFrame<SVec3>& other)
		{
			return left.tick < other.tick;
		});

	__debugbreak();

	return RuntimeAnimation{};
}
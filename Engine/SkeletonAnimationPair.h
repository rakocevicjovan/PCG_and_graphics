#pragma once

#include "Animation.h"
#include "Skeleton.h"


struct BoneChanelPair
{
	const Bone* bone;
	const AnimChannel* channel;
};


struct SkeletonAnimationPair
{
	std::vector<BoneChanelPair> _jointChannels;

	void create(const Skeleton& skeleton, const Animation& animation)
	{
		_jointChannels.reserve(skeleton.getBoneCount());

		for (const auto& bone : skeleton._bones)
		{
			// May be nullptr, this is expected. If they are all nullptr, it very likely indicates that the wrong animation or skeleton was passed.
			const AnimChannel* channel = animation.getAnimChannel(animation.getAnimChannelIndex(bone.name()));
			_jointChannels.emplace_back(BoneChanelPair{&bone, channel});
		}
	}
};
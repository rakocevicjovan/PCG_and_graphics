#pragma once

#include "TCachedLoader.h"
#include "Animation.h"
#include "Deserialize.h"


class AnimationManager final : public TCachedLoader<Animation, AnimationManager>
{
public:
	using TCachedLoader<Animation, AnimationManager>::TCachedLoader;

	// msvc please??
	//using TCachedLoader<Skeleton, SkeletonManager>::TCachedLoader<Skeleton, SkeletonManager>;

	Animation loadImpl(const char* path)
	{
		return AssetHelpers::DeserializeFromFile<Animation>(path);
	}
};
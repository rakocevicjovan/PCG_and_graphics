#pragma once

#include "TCachedLoader.h"
#include "Skeleton.h"
#include "Deserialize.h"


class SkeletonManager final : public TCachedLoader<Skeleton, SkeletonManager>
{
public:
	using base = TCachedLoader<Skeleton, SkeletonManager>;
	using TCachedLoader<Skeleton, SkeletonManager>::TCachedLoader;


	Skeleton loadImpl(const char* path)
	{
		return AssetHelpers::DeserializeFromFile<Skeleton>(path);
	}
};
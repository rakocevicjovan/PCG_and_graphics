#include "pch.h"
#include "Skeleton.h"


int Skeleton::getBoneIndex(const char* name) const
{
	for (auto i = 0; i < _bones.size(); ++i)
	{
		if (strcmp(_bones[i].name(), name) == 0)
		{
			return i;
		}
	}
	return -1;
}


int Skeleton::getInfluenceBoneIndex(const char* name) const
{
	int influenceIndex{ 0 };

	for (auto i = 0; i < _bones.size(); ++i)
	{
		if (strcmp(_bones[i].name(), name) == 0)
		{
			return influenceIndex;
		}

		influenceIndex += _bones[i].isInfluenceBone();
	}
	return -1;
}
#include "pch.h"
#include "Skeleton.h"


int Skeleton::getBoneIndex(const std::string& name) const
{
	for (int i = 0; i < _bones.size(); ++i)
	{
		if (_bones[i]._name == name)
		{
			return i;
		}
	}
	return -1;
}
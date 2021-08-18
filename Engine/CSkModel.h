#pragma once
#include "SkeletalModel.h"

struct CSkModel
{
	SkModel* skModel;

	operator SkModel& () { return *skModel; }
};
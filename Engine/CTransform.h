#pragma once
#include "Math.h"

struct CTransform
{
	SMatrix transform;

	operator SMatrix& () { return transform; }
};
#pragma once
#include "Math.h"

struct CTransform
{
	SMatrix transform;

	constexpr operator SMatrix& () { return transform; }
};
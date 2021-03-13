#pragma once
#include "Math.h"
#include <cstdint>

struct CTransform
{
	SMatrix transform;

	operator SMatrix&() { return transform; }
};
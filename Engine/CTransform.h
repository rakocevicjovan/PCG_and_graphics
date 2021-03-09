#pragma once
#include "Math.h"
#include <cstdint>

struct CTransform
{
	SMatrix transform;

	operator SMatrix&() { return transform; }
};

// Separate out later

struct CParentLink
{
	uint32_t parent;
};
#pragma once
#include <cstdint>

struct CParentLink
{
	uint32_t parent;

	operator uint32_t& () { return parent; }
};
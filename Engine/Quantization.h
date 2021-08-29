#pragma once
#include "pch.h"


// Expects min < max, does not check or assert for it, it's your responsibility to assure it.
template <typename InType, typename OutType>
inline OutType Quantize(InType in, InType min, InType max)
{
	static_assert(std::is_floating_point_v<InType>);
	static_assert(sizeof(InType) < sizeof(OutType));

	InType ratio = (in - min) / (max - min);

	OutType result = ratio * static_cast<OutType>(~0);
}


// Expects min < max, does not check or assert for it, it's your responsibility to assure it.
inline uint16_t QuantizeFToUi16(const float in, float min, float max)
{
	float ratio = (in - min) / (max - min);

	uint16_t result = ratio * static_cast<uint16_t>(~0);
}
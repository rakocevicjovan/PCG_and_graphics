#pragma once
#include <type_traits>

template <typename NumericType, unsigned int numElements>
struct NumericVector
{
	static_assert(std::is_arithmetic<NumericType> && numElements > 0);
	NumericType val[numElements];
};
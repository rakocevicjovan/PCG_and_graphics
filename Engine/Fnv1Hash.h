#pragma once
#include <string>
#include <cstdint>


// Magic numbers, magic magic numbers...
constexpr static uint32_t FNV1_OFFSET_BASIS = 0x811C9DC5;
constexpr static uint32_t FNV1_PRIME = 0x01000193;


static inline uint32_t fnv1hash(std::string str)
{
	uint32_t hash = FNV1_OFFSET_BASIS;
	unsigned int len = str.length();

	for (unsigned int i = 0; i < len; ++i)
	{
		hash *= FNV1_PRIME;
		hash ^= (str[i]);
	}

	return hash;
}

constexpr static inline uint32_t fnv1_CT(char const*const aString, const uint32_t val = FNV1_OFFSET_BASIS)
{
	return (aString[0] == '\0') ? val : fnv1_CT(&aString[1], (val * FNV1_PRIME) ^ uint32_t(aString[0]));
}

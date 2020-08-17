#pragma once
#include <string>
#include <cstdint>


// Magic numbers, magic magic numbers...
inline constexpr uint32_t FNV1_SEED = 0x811C9DC5;
inline constexpr uint32_t FNV1_PRIME = 0x01000193;


static inline uint32_t fnv1hash(const char* str)
{
	uint32_t hash = FNV1_SEED;
	const char* c = str;
	while(*c)
	{
		hash ^= *c++;
		hash *= FNV1_PRIME;
	}
	return hash;
}
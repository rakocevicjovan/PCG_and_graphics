#pragma once
#include <string>
#include <cstdint>

// Magic numbers, making magic numbers... take some magic then I put it in a number...
inline constexpr uint32_t FNV1_OFFSET_BASIS{ 0x811C9DC5 };
inline constexpr uint32_t FNV1_PRIME{ 0x01000193 };

static inline uint32_t fnv1hash(const char* str)
{
	uint32_t hash = FNV1_OFFSET_BASIS;
	const char* c = str;
	while(*c)
	{
		hash *= FNV1_PRIME;
		hash ^= *c++;
	}
	return hash;
}


inline constexpr uint64_t FNV1_OFFSET_BASIS_64{ 0xcbf29ce484222325 };
inline constexpr uint64_t FNV1_PRIME_64 = { 0x100000001b3 };

static inline uint64_t fnv1hash_64(const char* str)
{
	uint64_t hash = FNV1_OFFSET_BASIS_64;
	const char* c = str;
	while (*c)
	{
		hash *= FNV1_PRIME_64;
		hash ^= *c++;
	}
	return hash;
}

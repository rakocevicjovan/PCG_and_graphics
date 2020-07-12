#pragma once
#include <string>
#include <cstdint>

static uint32_t fnv1hash(std::string str)
{
	const uint32_t fnv_prime = 0x811C9DC5;
	uint32_t hash = 0;
	unsigned int len = str.length();

	for (unsigned int i = 0; i < len; ++i)
	{
		hash *= fnv_prime;
		hash ^= (str[i]);
	}

	return hash;
}
#pragma once

#include <vector>
#include <utility>
#include "Bits.h"


template<typename T, uint32_t lsb = 0, uint32_t msb = sizeof(T) * 8 - 1>
void CountingSort(T* input, uint32_t inputSize)
{
	constexpr uint32_t BitSpan = msb - lsb;
	constexpr uint32_t InputRange = 1 << BitSpan;
	constexpr T BitMask = bits::ContiguousBitmask<T, lsb, msb>::mask;

	uint32_t buckets[InputRange]{ 0u };	// Take this as input so it's only allocated once per radix.
		
	std::vector<T> output(inputSize, 0);

	T maxInVal = static_cast<T>(0u);	// Not generally true but will work for me... I'll only use ints/uints

	for (auto i = 0; i < inputSize; ++i)
	{
		const auto current = (input[i] & BitMask) >> lsb;
		++(buckets[current]);
		maxInVal = current > maxInVal ? current : maxInVal;
	}

	for (auto i = 1; i < maxInVal + 1; ++i)
	{
		buckets[i] += buckets[i - 1];
	}

	for (int i = inputSize - 1; i >= 0; --i)
	{
		const auto current = (input[i] & BitMask) >> lsb;
		output[--(buckets[current])] = input[i];
	}

	// Don't return a new vec for now, change in place
	std::copy(output.begin(), output.end(), input);
}


template <typename T, uint32_t bits_per_batch, uint32_t... int_seq>
void CountingSortRadixWrapper(T* inputArray, uint32_t inputSize, std::integer_sequence<uint32_t, int_seq...>)
{
	(CountingSort<T, int_seq* bits_per_batch, (int_seq + 1u) * bits_per_batch>(inputArray, inputSize), ...);
}


template <typename T, uint32_t bits_per_batch = 8u>
void RadixSort(T* inputArray, uint32_t inputSize)
{
	constexpr uint32_t numBatches{ (sizeof(T) * 8u) / bits_per_batch };
	constexpr std::integer_sequence int_seq = std::make_integer_sequence<uint32_t, numBatches>();

	CountingSortRadixWrapper<T, bits_per_batch>(inputArray, inputSize, int_seq);
}
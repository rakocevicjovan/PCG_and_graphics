#pragma once

#include "Bits.h"


template<typename T, uint32_t lsb = 0, uint32_t msb = sizeof(T) * 8>
void CountingSort(T* input, uint32_t inputSize)
{
	static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "This implementation of counting sort works only for unsigned integers.");

	static constexpr uint32_t BitSpan = msb - lsb;
	static constexpr uint32_t InputRange = 1 << BitSpan;
	static constexpr T BitMask = bits::ContiguousBitmask<T, lsb, msb>::mask;

	uint32_t count[InputRange]{ 0u };	// Take this as input so it's only allocated once per radix.
		
	std::vector<T> output(inputSize);

	T maxInVal = std::numeric_limits<T>::lowest();

	constexpr auto calculate_masked_value = [](T inputValue) constexpr -> T
	{
		if constexpr (lsb == 0 && msb == sizeof(T) * 8)
		{
			return inputValue;
		}
		else
		{
			return (inputValue & BitMask) >> lsb;
		}
	};

	for (auto i = 0; i < inputSize; ++i)
	{
		const auto current = calculate_masked_value(input[i]);
		++(count[current]);
		maxInVal = current > maxInVal ? current : maxInVal;
	}

	for (auto i = 1; i <= maxInVal; ++i)
	{
		count[i] += count[i - 1];
	}

	for (int i = inputSize - 1; i >= 0; --i)
	{
		const auto current = calculate_masked_value(input[i]);
		output[--(count[current])] = input[i];
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

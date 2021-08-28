#pragma once
#include "Math.h"
#include <cstdint>
#include <type_traits>
#include <tmmintrin.h>


// For now it's only enabled for float/double input and integral output
namespace quatCompression
{
	// Creates a bitmask with bits [msb, lsb) set to 1, others set to 0. Zero indexed. Can be used at runtime
	template <typename IntegralType>
	constexpr IntegralType CreateContiguousBitmask(uint32_t lsb, uint32_t msb)
	{
		static_assert(std::is_integral_v<IntegralType>);
		constexpr uint32_t typeBitSize = sizeof(IntegralType) * 8u;
		auto result{ static_cast<IntegralType>(~0u) << lsb };
		return result & (result >> (typeBitSize - msb));
	}

	// Compile time mask
	template <typename IntegralType, IntegralType lsb, IntegralType msb>
	struct ContiguousBitmask
	{
	private:
		static_assert(std::is_integral_v<IntegralType>);
		static constexpr uint32_t typeBitSize = sizeof(IntegralType) * 8u;
		static constexpr IntegralType maxVal = static_cast<IntegralType>(~0u);
	public:
		static constexpr IntegralType mask{ (maxVal << lsb) & (maxVal >> (typeBitSize - msb)) };
	};


	// Quaternion (de)compression constants
	constexpr float maxQuatValue = 0.70710678118;	// Since sqrt() isn't constexpr: 1. / sqrt(2.) = 0.70710678118 
	constexpr float minQuatValue = -maxQuatValue;
	constexpr float valRange = 2. * maxQuatValue;

	constexpr uint32_t outComponentBitWidth = 10u;
	constexpr uint32_t resultMaxVal = 1u << outComponentBitWidth - 1;


	// Tested and passing when first written.
	SQuat Decompress(uint32_t compressedQuat)
	{
		const uint32_t unpacked_index = compressedQuat >> 30;

		constexpr auto decompress =
			[](uint32_t unpackedInt) -> float
			{
				return minQuatValue + (unpackedInt * 2.0 / resultMaxVal) * maxQuatValue;
			};

		float result[4]{0.f};
		float total{ 0.f };
		total += (result[0 + (unpacked_index == 0)] = decompress((compressedQuat & ContiguousBitmask<uint32_t, 00, 10>::mask) >> 00));
		total += (result[1 + (unpacked_index <= 1)] = decompress((compressedQuat & ContiguousBitmask<uint32_t, 10, 20>::mask) >> 10));
		total += (result[2 + (unpacked_index <= 2)] = decompress((compressedQuat & ContiguousBitmask<uint32_t, 20, 30>::mask) >> 20));
		result[unpacked_index] = sqrt(1. - static_cast<SVec4>(result).Dot(static_cast<SVec4>(result)));

		return static_cast<SQuat>(result);
	}


	inline uint8_t findMaxQuatVal(const SQuat& quat)
	{
		uint8_t maxValIndex{ 0u };

		float maxVal = fabs(quat.x);

		for (auto i = 1; i < 4; ++i)
		{
			const float current = fabs((&quat.x)[i]);
			if (current > maxVal)
			{
				maxVal = current;
				maxValIndex = i;
			}
		}

		return maxValIndex;
	}


	uint32_t compressQuaternion(SQuat& quat)
	{
		uint8_t maxValIndex = findMaxQuatVal(quat);

		quat = static_cast<float*>(&quat.x)[maxValIndex] > 0.f ? quat : -quat;

		uint32_t packingIndex{ 0u };
		uint32_t result{ 0u };

		for (auto i = 0; i < 4; ++i)
		{
			// Maximum value is discarded
			if (i == maxValIndex)
				continue;
			
			const float remappedElement = ((&quat.x)[i] - minQuatValue) / (valRange);
			const auto quantized = static_cast<uint32_t>(remappedElement * resultMaxVal);
			result |= quantized << (packingIndex++ * outComponentBitWidth);
		}

		result |= maxValIndex << 30;

		return result;
	}
}


/* 
// Afaik this works but I have no use for it RN as I needed the index
// Input must be a contiguous array of 4 floats (SVec4, SQuat etc...)
inline void simdMax(float* inout)
{
	// Can skip a load here since it's aligned already
	__m128& simdVal = (__m128&)(inout);
	// Shuffle 3 times to compare every element with other elements, result is the max value in all 4 floats
	for (int i = 0; i < 3; i++)
		simdVal = _mm_max_ps(simdVal, _mm_shuffle_ps(simdVal, simdVal, _MM_SHUFFLE(2, 1, 0, 3)));
}
*/


/*
// Keep this version too. It looks slower to me but may actually not be!
SQuat testDecompress(uint32_t compressedQuat)
{
	const uint32_t unpacked_index = compressedQuat >> 30;
	SQuat unpacked{};
	float total{ 0.f };

	auto realUpkIndex{ 0u };
	for (auto i = 0; i < 4; ++i)
	{
		i += (i == unpacked_index);

		uint32_t bitmaskI = CreateContiguousBitmask<uint32_t>(realUpkIndex, realUpkIndex + outComponentBitWidth);
		uint32_t unpackedInt = (compressedQuat & bitmaskI) >> realUpkIndex;

		float& floatI = (&unpacked.x)[i];
		floatI = -maxQuatValue + (unpackedInt * 2.0 / resultMaxVal) * maxQuatValue;

		total += floatI * floatI;

		realUpkIndex += outComponentBitWidth;
	}

	(&unpacked.x)[unpacked_index] = sqrt(1. - total);

	return unpacked;
}
*/
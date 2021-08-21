#pragma once
#include "Math.h"
#include <cstdint>
#include <type_traits>
#include <tmmintrin.h>


// For now it's only enabled for float/double input and integral output
namespace quantization
{
	template <typename ValueType>
	struct QuantSpan
	{
		static_assert(std::is_floating_point_v<ValueType>);
		ValueType min{};
		ValueType max{};

		constexpr QuantSpan() noexcept = default;
		constexpr QuantSpan(ValueType min, ValueType max) noexcept : min(min), max(max) {}
	};


	template <typename OutputType, typename ValueType>
	inline constexpr OutputType quantize(ValueType value, const QuantSpan<ValueType>& span, uint32_t numBits)
	{
		static_assert(std::is_integral_v<OutputType>);

		const auto range = span.max - span.min;

		const double granularity = static_cast<double>(2u << numBits);
		const double step = range / granularity;
	}


	// Creates a bitmask with bits (msb, lsb) set to 1, others set to 0. Zero indexed.
	template <typename IntegralType>
	constexpr IntegralType CreateContiguousBitmask(uint32_t lsb, uint32_t msb)
	{
		static_assert(std::is_integral_v<IntegralType>);

		constexpr uint32_t typeBitSize = sizeof(IntegralType) * 8u;

		auto result{ static_cast<IntegralType>(~0u) << lsb };
		return result & (result >> (typeBitSize - msb));
	}


	uint32_t compressQuaternion(SQuat& quat)
	{
		SQuat positiveWQuat = quat.w > 0.f ? quat : quat * (-1.f);
		
		uint8_t maxValIndex{ 0 };
		
		{
			float maxVal = quat.x;
			for (auto i = 1; i < 4; ++i)
			{
				const float current = fabs((&quat.x)[i]);
				if (current > maxVal)
				{
					maxVal = current;
					maxValIndex = i;
				}
			}
		}

		// Since sqrt() isn't constexpr: 1. / sqrt(2.) = 0.70710678118 
		constexpr float maxQuatValue = 0.70710678118;
		constexpr float valRange = 2. * maxQuatValue;

		constexpr uint32_t outComponentBitWidth = 10u;
		constexpr uint32_t resultMaxVal = 1u << outComponentBitWidth - 1;

		uint32_t packingIndex{ 0u };
		uint32_t result{ 0u };

		for (auto i = 0; i < 4; ++i)
		{
			// Maximum value is discarded
			if (i == maxValIndex)
				continue;
			
			const float remappedElement = ((&quat.x)[i] - (-maxQuatValue)) / (valRange);

			const uint32_t quantized = static_cast<uint32_t>(remappedElement * resultMaxVal);
			
			float unpacked = -maxQuatValue + ( ((quantized * 2.) / resultMaxVal) * maxQuatValue );

			result |= quantized << (packingIndex++ * outComponentBitWidth);
		}

		result |= maxValIndex << 30;

		// This tests it worked. It was passing when implemented.
		//testDecompress();

		return result;
	}


	// Should pass
	SQuat testDecompress(uint32_t compressedQuat)
	{
		constexpr float maxQuatValue = 0.70710678118;
		constexpr float valRange = 2. * maxQuatValue;

		constexpr uint32_t outComponentBitWidth = 10u;
		constexpr uint32_t resultMaxVal = 1u << outComponentBitWidth - 1;

		const uint32_t unpacked_index = compressedQuat >> 30;

		SQuat unpacked{};
		float total{ 0.f };

		auto realUpkIndex{ 0u };
		for (auto i = 0; i < 4; ++i)
		{
			//bool isBiggest = ;
			i += (i == unpacked_index);

			uint32_t bitmaskI = CreateContiguousBitmask<uint32_t>(realUpkIndex, realUpkIndex + 10);
			uint32_t unpackedInt = (compressedQuat & bitmaskI) >> realUpkIndex;

			float& floatI = (&unpacked.x)[i];
			floatI = -maxQuatValue + (unpackedInt * 2.0 / resultMaxVal) * maxQuatValue;

			total += floatI * floatI;

			realUpkIndex += 10;
		}

		(&unpacked.x)[unpacked_index] = sqrt(1. - total);

		return unpacked;
	}
}


/* Afaik this works but I have no use for it RN as I needed the index
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
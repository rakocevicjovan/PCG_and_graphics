#include "pch.h"

#include "LZ4Wrapper.h"
#include "lz4.h"
#include "Blob.h"


namespace lz4
{
	int compress(const char* input, uint32_t inSize, char* output, uint32_t outSize)
	{
		int result = LZ4_compress_default(input, output, inSize, outSize);

		return result;
	}


	Blob compressToNewBuffer(const char* input, uint32_t inSize)
	{
		auto worstCaseOutputSize = predictMaxOutSize(inSize);

		std::unique_ptr<char[]> outputBuffer = std::make_unique<char[]>(worstCaseOutputSize);

		int result = LZ4_compress_default(input, outputBuffer.get(), inSize, worstCaseOutputSize);

		return Blob{ std::move(outputBuffer), static_cast<size_t>(worstCaseOutputSize) };
	}


	int predictMaxOutSize(uint32_t inputSize)
	{
		return LZ4_compressBound(inputSize);
	}
}
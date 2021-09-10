#pragma once


namespace lz4
{
	int compress(const char* input, uint32_t inSize, char* output, uint32_t outSize);
	Blob compressToNewBuffer(const char* input, uint32_t inSize);

	int predictMaxOutSize(uint32_t inputSize);
}
#pragma once
#include "stb_image.h"
#include "stb_image_write.h"

class Image;

class STBImporter
{
private:

	struct STB_info
	{
		int width;
		int height;
		int srcChannels;
		int wantedChannels;
	};

public:

	static STB_info GetFormatFromFile(const char* filename);
	static STB_info GetFormatFromMemory(const unsigned char* data, size_t size);

	static Image ImportFromFile(const char* filename);
	static Image ImportFromMemory(const unsigned char* data, size_t size);

	static std::vector<float> ImportFloatVector(const char* path);

	static bool SaveAsPNG(const char* outputPath, int w, int h, int numChannels, void* data, uint32_t stride = 0u);
};
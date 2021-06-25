#include  "pch.h"

#include "STBImporter.h"
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#define STBI_MSC_SECURE_CRT
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "stb_image.h"
#include "stb_image_write.h"


STBImporter::STB_info STBImporter::GetFormatFromFile(const char* filename)
{
	int w, h, n;
	stbi_info(filename, &w, &h, &n);
	return STBImporter::STB_info { w, h, n, n == 3 ? 4 : n };
}


STBImporter::STB_info STBImporter::GetFormatFromMemory(const unsigned char* data, size_t size)
{
	int w, h, n;
	stbi_info_from_memory(data, size, &w, &h, &n);
	return STBImporter::STB_info{ w, h, n, n == 3 ? 4 : n };
}


Image STBImporter::ImportFromFile(const char* fileName)
{
	assert(fileName);
	STB_info info = GetFormatFromFile(fileName);

	auto decodedData = std::shared_ptr<uint8_t[]>(stbi_load(fileName, &info.width, &info.height, &info.srcChannels, info.wantedChannels));

	if (!decodedData)
	{
		OutputDebugStringA("Error loading texture from file. \n");
		return Image{};
	}

	return Image(info.width, info.height, decodedData.get(), info.wantedChannels, fileName);
}


Image STBImporter::ImportFromMemory(const unsigned char* data, size_t size)
{
	assert(data);
	STB_info info = GetFormatFromMemory(data, size);

	// This could be dangerous. If something throws here it could leak memory. 
	// However the image constructor is probably not going to since it just takes ownership of data and doesnt allocate.
	auto decodedData = stbi_load_from_memory(data, size, &info.width, &info.height, &info.srcChannels, info.wantedChannels);	// std::shared_ptr<unsigned char[]>()

	if (!decodedData)
	{
		OutputDebugStringA("Error loading texture from memory. \n");
		return Image{};
	}

	return Image(info.width, info.height, decodedData, info.wantedChannels);
}


bool STBImporter::SaveAsPNG(const char* outputPath, int w, int h, int numChannels, void* data, uint32_t stride)
{
	return stbi_write_png(outputPath, w, h, numChannels, data, stride);
}


// Staying as it is to avoid reworking strife level but should remove the copy.
std::vector<float> STBImporter::ImportFloatVector(const char* path)
{
	STB_info info = GetFormatFromFile(path);
	auto floats = std::unique_ptr<float[]>(stbi_loadf(path, &info.width, &info.height, &info.srcChannels, 4));

	if (!floats)
	{
		OutputDebugStringA("Error loading floats from file. \n");
		return {};
	}

	std::vector<float> result(floats.get(), floats.get() + info.width * info.height * 4);
	return std::move(result);
}
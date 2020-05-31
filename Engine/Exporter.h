#pragma once
#include "AssimpWrapper.h"
#include "FileUtilities.h"



class Exporter
{
public:

	static void exportTexture(const Texture& texture, std::string& exportPath)
	{
		// Metadata 4 * 4 bytes, data varies
		int w = texture.getW();
		int h = texture.getH();
		int n = texture.getN();
		TextureRole role = texture._role;

		const unsigned char* data = texture.getData();

		UINT metadataSize = 16;
		UINT dataSize = w * h * n;

		char* output = new char[metadataSize + dataSize];
		memcpy(&output, &w, metadataSize);
		memcpy(&output + 16, data, dataSize);

		FileUtils::writeAllBytes(exportPath.c_str(), &output, metadataSize + dataSize);

		// Too naive, needs to be replicated otherwise
		//FileUtils::writeAllBytes(exportPath.c_str(), &texture, sizeof(Texture));
	}
};
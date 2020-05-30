#pragma once
#include "AssimpWrapper.h"
#include "FileUtilities.h"



class Exporter
{
	std::string _exportPath;


	static void exportTexture(const Texture& texture, std::string& exportPath)
	{
		// Bit by bit
		int w = texture.getW();
		int h = texture.getH();
		int n = texture.getN();

		const unsigned char* data = texture.getData();

		// Too naive, needs to be replicated otherwise
		//FileUtils::writeAllBytes(exportPath.c_str(), &texture, sizeof(Texture));
	}
};
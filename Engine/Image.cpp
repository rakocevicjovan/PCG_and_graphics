#include "pch.h"
#include "Image.h"

#include "STBImporter.h"


void Image::saveAsPng(const char* outputPath)
{
	STBImporter::SaveAsPNG(outputPath, _width, _height, _numChannels, _data.get());
}


void Image::loadFromStoredPath()
{
	*this = STBImporter::ImportFromFile(_fileName.c_str());
}
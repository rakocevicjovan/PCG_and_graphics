#pragma once
#include "stb_image.h"
#include "stb_image_write.h"


class Image
{
private:

	// Helpful for debug, likely to be removed in release...
	std::string _fileName;

	uint32_t _width{};
	uint32_t _height{};

	uint8_t _numChannels{};

	std::vector<uint8_t> _data;

public:

	inline auto width() const { return _width; };
	inline auto height() const { return _height; };

	void releaseData()
	{
		_data.clear();
	}

	void saveAsPng()
	{
		/*try
		{
			int result = stbi_write_png(targetFile, w, h, comp, data, stride_in_bytes);
		}
		catch (...)
		{
			std::string errorMessage = "Error writing texture to file: ";
			errorMessage += targetFile;
			OutputDebugStringA(errorMessage.c_str());
		}*/
	}


	bool loadFromStoredPath()
	{
		assert(false);	// Not impl yet
		try
		{
			//loadFromFile(_fileName.c_str());
			return (_data.size());
		}
		catch (...)
		{
			OutputDebugStringA(("Error loading texture '" + _fileName + "' \n").c_str());
			return false;
		}
	}


	// These functions are unoptimized and slow, not intended for use per frame
	//for comp: 1=Y, 2=YA, 3=RGB, 4=RGBA 
	static void SaveAsPng(const char* targetFile, int w, int h, int comp, const void* data, uint32_t byteStride = 0u)
	{
		try
		{
			int result = stbi_write_png(targetFile, w, h, comp, data, byteStride);
		}
		catch (...)
		{
			std::string errorMessage = "Error writing texture to file: ";
			errorMessage += targetFile;
			OutputDebugStringA(errorMessage.c_str());
		}
	}

};
#pragma once


enum class TexelFormat : uint8_t
{
	// idk stuff
};


class Image
{
private:

	uint32_t _width{};
	uint32_t _height{};

	uint8_t _numChannels{};

	std::shared_ptr<unsigned char[]> _data;

	// Helpful for debug, likely to be removed in release...
	std::string _fileName;

public:

	Image() = default;

	Image(uint32_t width, uint32_t height, unsigned char* data, uint8_t texel_width, const char* name = "")
		: _width(width), _height(height), _fileName(name), _numChannels(texel_width), _data(std::shared_ptr<unsigned char[]>(data))
	{}

	void saveAsPng(const char* outputPath);
	void loadFromStoredPath();

	inline auto width() const { return _width; }
	inline auto height() const { return _height; }
	inline auto numChannels() const { return _numChannels; }
	inline const std::string& getPath() const { return _fileName; }

	inline const auto& data() const { return _data; }
	inline unsigned char* dataRaw() { return _data.get(); }
	inline void releaseData() { _data.reset(); }
};
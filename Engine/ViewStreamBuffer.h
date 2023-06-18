#pragma once

// A hack around cereal only taking std::istream for input archives. Do NOT use for writing!
class ViewStreamBuffer final : public std::streambuf
{
public:

	ViewStreamBuffer(char* data, size_t size)
	{
		this->setg(data, data, data + size);
	}

	virtual int overflow(int a) override	// Put a character on the stream
	{
		assert(false && "Do not use ViewStreamBuffer for writing!");
		return 0;
	}
};

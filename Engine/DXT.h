#pragma once
// By graphitemaster, license at https://github.com/graphitemaster/gmrtdxt/blob/master/LICENSE

enum dxtType {
	kDXT1,
	kDXT5
};

template <dxtType T>
std::vector<unsigned char> dxtCompress(const unsigned char *const uncompressed,
	size_t width, size_t height, size_t channels, size_t &outSize, size_t &optimizedBlocks);
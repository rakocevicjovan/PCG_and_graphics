#include "VertSignature.h"
#include <cassert>


void VertSignature::addAttribute(VAttrib attrib)
{
	_attributes.push_back(attrib);
}



void VertSignature::addAttribute(VAttribSemantic s, VAttribType t, uint8_t size, uint8_t numElements)
{
	_attributes.emplace_back(s, t, size, numElements);
}



uint16_t VertSignature::getVertByteWidth()
{
	uint16_t total{0u};

	for (auto attrib : _attributes)
		total += (attrib._size * attrib._numElements);

	return total;
}



uint16_t VertSignature::getOffsetOf(VAttribSemantic semantic, uint8_t index)
{
	uint16_t offset{ 0u };

	for (auto attr : _attributes)
	{
		// Account for something like above, {TEX_COORD, float2, 4}, return offset to TEX_COORD + 
		if (attr._semantic == semantic)
		{
			offset += index * attr._size;
			break;
		}
		offset += attr._size * attr._numElements;
	}

#ifdef _DEBUG
	assert((offset != getVertByteWidth()) && "Vertex attribute not found.");
#endif

	return offset;
}


// A bit stupid, but it's not a big deal
// Typeless stores 0 therefore a proper size must be passed in.
const std::vector<uint16_t> VAttrib::VERT_TYPE_SIZE =
{
	4, 8, 16, 32,
	4, 8, 16, 32,
	4, 8, 16, 32,
	0
};
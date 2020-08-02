#include "VertSignature.h"
#include <cassert>


void VertSignature::addAttribute(VAttrib attrib)
{
	_attributes.push_back(attrib);
}



void VertSignature::addAttribute(VAttribSemantic s, VAttribType t, uint8_t numElements, uint8_t elemByteSize)
{
	_attributes.emplace_back(s, t, numElements, elemByteSize);
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
		// Account for something like above, {TEX_COORD, float2, 4}, return offset to TEX_COORD[4]
		if (attr._semantic == semantic)
		{
#ifdef _DEBUG 
			assert(index < attr._numElements && "Attribute index too high.");
#endif
			offset += index * attr._size;
			break;
		}
		offset += attr._size * attr._numElements;
	}

#ifdef _DEBUG
	//assert(offset != getVertByteWidth() && "Vertex attribute not found.");
#endif

	return offset;
}



std::vector<D3D11_INPUT_ELEMENT_DESC> VertSignature::createVertInLayElements()
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> vertInLayElements;

	for (UINT i = 0; i < _attributes.size(); ++i)
	{
		const auto& attr = _attributes[i];

		for (UINT j = 0; j < attr._numElements; ++j)
		{
			vertInLayElements.push_back({
				VERTSIG_TO_SEMANTIC[static_cast<uint8_t>(attr._semantic)],
				j,
				VERTSIG_TO_DXGIFORMAT[static_cast<uint8_t>(attr._type)],
				0u,	// Which buffer is this in, @TODO
				D3D11_APPEND_ALIGNED_ELEMENT,	// Automagical...
				D3D11_INPUT_PER_VERTEX_DATA,
				0u								// Always 0 for per vertex data
				});
		}
	}
	return vertInLayElements;
}


// A bit strange as many are of the same size, but it's not a big deal
// Typeless stores 0 therefore a proper size must be passed in.
const std::vector<uint16_t> VAttrib::VERT_TYPE_SIZE =
{
	4, 8, 12, 16,
	4, 8, 12, 16,
	4, 8, 12, 16,
	0
};
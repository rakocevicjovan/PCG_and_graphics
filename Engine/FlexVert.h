#pragma once
#include "VertSignature.h"
#include <memory>

class FlexVertArray
{
public:
	
	std::unique_ptr<char[]> _data;
	VertSignature _signature;
	uint32_t _numVerts;

	FlexVertArray(VertSignature signature, uint32_t numElements)
		: _signature(signature), _numVerts(numElements)
	{
		_data = std::make_unique<char[]>(numElements);
		sizeof(FlexVertArray);
	}

};
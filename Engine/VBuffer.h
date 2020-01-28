#pragma once
#include "MeshDataStructs.h"

class VBuffer
{
protected:
	ID3D11Buffer* _vertexBuffer = nullptr;

public:

	VBuffer() {}

	VBuffer(UINT stride, UINT offset) : _stride(stride), _offset(offset) {}

	inline ID3D11Buffer* const * ptr() const { return &_vertexBuffer; }
	inline ID3D11Buffer*& ptrVar() { return _vertexBuffer; }

	UINT _stride = sizeof(Vert3D);
	UINT _offset = 0;
};
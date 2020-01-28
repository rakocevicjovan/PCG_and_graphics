#pragma once


class IBuffer
{
protected:
	ID3D11Buffer* _indexBuffer = nullptr;

public:

	inline ID3D11Buffer* ptr() const { return _indexBuffer; }
	inline ID3D11Buffer*& ptrVar() { return _indexBuffer; }
};
#pragma once


class IBuffer
{
protected:
	ID3D11Buffer* _indexBuffer = nullptr;
	UINT _count;
public:

	inline ID3D11Buffer* ptr() const { return _indexBuffer; }
	inline ID3D11Buffer*& ptrVar() { return _indexBuffer; }

	inline UINT getIdxCount() { return _count; }
	inline void setIdxCount(UINT count) { _count = count; }
};
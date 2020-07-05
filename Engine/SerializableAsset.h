#pragma once
#include <memory>

typedef unsigned int UINT;

// Almost a stack allocator, isn't it? ADD WILL FAIL for c-style arrays though...
struct MemChunk
{
	std::unique_ptr<char[]> _ptr;
	UINT _size;

	MemChunk() : _size(0) {};

	MemChunk(std::unique_ptr<char[]> ptr, UINT size) : _ptr(std::move(ptr)), _size(size) {}

	MemChunk(UINT size) : _ptr(std::unique_ptr<char[]>(new char[size]())), _size(size) {}
	
	

	template <typename Datum> 
	inline void add(const Datum* datum, UINT& offset)
	{
		write(datum, sizeof(Datum), offset);
	}

	template <typename VecData>
	inline void add(const std::vector<VecData>& data, UINT& offset)
	{
		write(data.data(), sizeof(VecData) * data.size(), offset);
	}


	void write(const void* data, UINT cpySize, UINT& offset)
	{
		UINT newSize = offset + cpySize;

		if (newSize > _size)
		{
			char errStr[150];
			sprintf(errStr, 
				"Serializing overflow. Available: %d; \n"
				"Attempted to add: %d \n"
				"For a total of: %d \n",
				_size, cpySize, newSize);

			exit(7645);	// Let's pretend I actually have an error code table
		}

		memcpy(_ptr.get() + offset, data, cpySize);
		offset = newSize;
	}


	inline bool isFull(UINT offset)
	{
		return (offset == _size);
	}


	inline void validateSize(UINT offset)
	{
		if (!isFull(offset))
		{
			OutputDebugStringA("SERIALIZATION WARNING: SIZE MISMATCH!");
			throw;
		}
	}
};



class SerializableAsset
{
public:

	virtual MemChunk Serialize() = 0;
};
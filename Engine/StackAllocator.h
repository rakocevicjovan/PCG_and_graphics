#pragma once
#include <cstdint>

class StackAllocator
{
	typedef uint8_t byte;

private:

	byte* _stackPtr;
	size_t _stackSize;

	uint32_t _head;

public:

	explicit StackAllocator()			//(size_t size) : _stackSize(size)
	{
		_stackPtr = nullptr;
	}

	StackAllocator(const StackAllocator&) = delete;

	StackAllocator& operator=(const StackAllocator&) = delete;

	~StackAllocator()
	{
		if(_stackPtr)
			delete[] _stackPtr;
	}



	inline void init(size_t size)
	{
		_stackSize = size;
		_stackPtr = new byte[_stackSize];
		_head = 0;
	}



	inline byte* alloc(size_t size)
	{
		byte* toReturn = _stackPtr + _head;		//will return the pointer to the current head offset
		_head += size;							//will increment the head offset for future allocations to not overwrite this
		return toReturn;						//this pointer is used to rewind the stack and "free" memory
	}



	inline void rewind(byte* target)
	{
		//_stackPtr = target;
		_head = target - _stackPtr;
	}



	inline byte* getHeadPtr() const
	{
		return _stackPtr + _head;
	}


	//DOES NOT FREE ANY EXTERNALLY ALLOCATED MEMORY!!! THAT'S UP TO THE DESTRUCTOR - EX: MESH HAS VECTORS
	inline void clear()
	{
		_head = 0;
	}


	inline byte* getStackPtr() const
	{
		return _stackPtr;
	}

};
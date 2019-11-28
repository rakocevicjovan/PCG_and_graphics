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
			delete _stackPtr;
	}



	void init(size_t size)
	{
		_stackSize = size;
		_stackPtr = new byte[_stackSize];
		_head = 0;
	}



	byte* alloc(size_t size)
	{
		byte* toReturn = _stackPtr + _head;		//will return the pointer to the current head offset
		_head += size;							//will increment the head offset for future allocations to not overwrite this
		return toReturn;						//this pointer is used to rewind the stack and "free" memory
	}



	void rewind(byte* target)
	{
		_stackPtr = target;
	}



	byte* getHead()
	{
		return _stackPtr + _head;
	}


	//DOES NOT FREE MEMORY!!! THAT'S UP TO THE DESTRUCTOR
	void clear()
	{
		_head = 0;
	}

};
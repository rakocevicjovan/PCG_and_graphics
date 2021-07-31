#pragma once
#include <cstdint>


// CLEAR DOES NOT FREE ANY EXTERNALLY ALLOCATED MEMORY!!! AS SIMPLE AS POSSIBLE TO HOLD PODs ONLY!
class StackAllocator
{
	typedef uint8_t byte;

private:

	byte* _stackPtr;
	uint32_t _stackSize;

	uint32_t _head;

public:

	explicit StackAllocator() : _stackPtr(nullptr), _stackSize(0u), _head(0u) {}

	StackAllocator(const StackAllocator&) = delete;

	StackAllocator& operator=(const StackAllocator&) = delete;

	~StackAllocator()
	{
		if(_stackPtr)
			delete[] _stackPtr;
	}



	inline void init(uint32_t size)
	{
		_stackSize = size;
		_stackPtr = new byte[_stackSize];
		_head = 0u;
	}



	inline byte* alloc(uint32_t size)
	{
		byte* address = getHeadPtr();	// Return the pointer to the current head offset
		_head += size;					// Increment the head offset
		return address;					// Can be used to rewind, but risky.
	}


	// Use at your own risk, must understand how stacks work. Still, it's an option
	inline void rewind(byte* target)
	{
		_head = target - _stackPtr;
	}



	inline byte* getHeadPtr() const
	{
		return _stackPtr + _head;
	}



	inline void clear()
	{
		_head = 0;
	}


	inline const byte* const getStackPtr() const
	{
		return _stackPtr;
	}
};
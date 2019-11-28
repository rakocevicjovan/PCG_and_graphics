#pragma once
#include <cstdint>

class StackAllocator
{
	typedef uint8_t byte;

private:
	byte* _stack;
	uint32_t _head;

public:

	explicit StackAllocator(size_t size)
	{
		_stack = new byte[size];
		_head = 0;
	}



	~StackAllocator()
	{
		if(_stack)
			delete _stack;
	}


	byte* alloc(size_t size)
	{
		_stack += size;
	}



	void rewind(byte* target)
	{
		_stack = target;
	}



	uint32_t getHead()
	{
		return _head;
	}



	void clear()
	{
		_head = 0;
	}

};
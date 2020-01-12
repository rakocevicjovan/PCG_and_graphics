#pragma once
#include <vector>
#include <list>



struct ID
{
	uint32_t _index;		//over 4 billion slots, good luck running out... I guess?
	uint32_t _internal;
};


//not implemented, do not use yet, @TODO
template <typename Object> class SlotVector
{
private:
	std::vector<Object> _vec;
	std::list<uint32_t> _freeList;

public:

	void create(uint32_t size)
	{
		_vec.resize(size);
	}


	void pushToFree(Object obj)
	{
		_vec[_freeList.front()] = obj;
	}


	void eraseByIndex(uint32_t index)
	{
		_vec[index];
	}
};
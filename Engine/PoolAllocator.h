#pragma once
#include <stdint.h>
#include <assert.h>
#include <new>

typedef unsigned int UINT;
typedef uint8_t BYTE;


struct Slot
{
	Slot* _next;	
};


//does NOT call destructors of objects - use for POD-s
template <class Object> class PoolAllocator
{
private:

	PoolAllocator(const PoolAllocator& other) = delete;
	PoolAllocator& operator=(const PoolAllocator&) = delete;


protected:

	BYTE* _pool;		//ptr to beginning of the pool
	Slot* _free;		//ptr to first free slot
	UINT _capacity;		//n of provided objects to allocate memory for, not the entire memory


public:
	
	PoolAllocator(UINT objCapacity) : _capacity(objCapacity)
	{
		//check if we can store the freelist member instead of an object
		static_assert(sizeof(Object) >= sizeof(void*), "Object size too small.");

		//allocate enough memory for n objects of provided type
		UINT poolSize = objCapacity * sizeof(Object);

		_pool = new BYTE[poolSize];
		
		reset();
	}



	~PoolAllocator()
	{
		delete[] _pool;
	}



	inline Object* allocate()
	{
		//a bit aggressive but I'd rather know right away
		assert(_free, "Pool allocator overflow.");

		Object* result = reinterpret_cast<Object*>(_free);

		_free = _free->_next;	//advance the freelist pointer;

		return result;
	}



	inline void deallocate(Object* object)
	{
		reinterpret_cast<Slot*>(object)->_next = _free;
		_free = reinterpret_cast<Slot*>(object);
	}
	


	void reset()
	{
		Slot* current = new (_pool) Slot;

		//create free list members, these fill each pool allocator slot initially
		for (uint32_t i = 1; i < _capacity; i++)
		{
			//Object* tObj = reinterpret_cast<Object*>(&_pool[i * sizeof(Object)]);
			void* tObj = &_pool[i * sizeof(Object)];
			current->_next = new (tObj) Slot;
			current = current->_next;
		}

		//last slot has no _next
		current->_next = nullptr;

		_free = reinterpret_cast<Slot*>(_pool);
	}
};


/*
//~40ms to create pool for a million SphereHull objects
//~30 ms to allocate a million pointers (no construction)
//~152 ms vs ~300 ms to allocate (and construct! fair comparison against new) with the pool vs using new each time
//decent speedup, could possibly be faster?
//test code for reference
	UINT B33G = 1000000;
	PoolAllocator<SphereHull> pasp(B33G);

	for (UINT i = 0; i < B33G; i++)
	{
		SphereHull* sp = new (pasp.allocate()) SphereHull(SVec3(10), 5.f);
	}

	for (UINT i = 0; i < B33G; i++)
	{
		SphereHull* sp = new SphereHull(SVec3(10), 5.f);
	}
*/
#pragma once
#include <vector>
#include <assert.h>

typedef unsigned int UINT;
typedef uint16_t Handle;


// Preallocated vector that keeps contiguous elements using "swapping" (not really but close) and an index lookup vector
// Allows contiguous memory allocation with stable references to vector elements
// WARNING! Erased element handles are NOT... well, handled. If you call erase(), make sure to not use the handle any more!
template <typename Object> class SlotVector
{
private:

	std::vector<Object> _objects;
	std::vector<uint16_t> _indices;	// Index of this vector is the handle, value is the actual index in _objects array
	uint16_t _firstFree;
	uint16_t _capacity;

public:

	SlotVector(uint16_t numObjects) : _capacity(numObjects), _firstFree(0u)
	{
		_objects.resize(numObjects);
		_indices.resize(numObjects);
	}



	Handle insert(const Object& o)
	{
		//Crash or adapt... hmmm? Adapting silently could be very bad in case of a bug somewhere, size is limited for a reason.
		if (_firstFree >= _capacity)
		{
			assert(false && "Slot vector - attempted insert over capacity.");
			//_capacity <<= 1;
			//_objects.resize(_capacity);
			//_indices.resize(_capacity);
		}

		Handle result = _firstFree;			// index of first free element
		_objects[_firstFree] = o;			// insert object into array		
		_indices[_firstFree] = _firstFree;	// index vector updated, contains real index of object (they are same for now)
		_firstFree++;						// free index increments

		return result;
	}



	void erase(Handle h)
	{
		if (h >= _firstFree)
			return;

		uint16_t objIndex = _indices[h];
		--_firstFree;
		_objects[objIndex] = _objects[_firstFree];		// overwrite erased element with last element
		_indices[_firstFree] = objIndex;				// last element also claims the index to keep it synchronized
	}



	inline Object* getPtrByHandle(Handle h)
	{
		return h >= _firstFree ? nullptr : &(_objects[_indices[h]]);	// Double indirection but it shouldn't be used often anyways!
	}


	//Remember that size is not necessarily the number of used slots, instead use getNumAllocated to iterate
	inline std::vector<Object>& getObjectVector()
	{
		return _objects;
	}



	inline uint16_t getNumAllocated() const
	{
		return _firstFree;
	}
};
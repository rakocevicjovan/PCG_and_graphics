#pragma once
#include <vector>
#include <algorithm>
#include <assert.h>

typedef unsigned int UINT;
typedef uint16_t Handle;

// THIS IS NOT FINISHED AND _firstFree is absolutely not working right now.

// Preallocated vector that keeps contiguous elements using "swapping" (not really but close) and an index lookup vector
// Allows contiguous memory allocation with stable references to vector elements
// Using the handle after erase() is undefined.
template <typename Object> class SparseSet
{
private:

	struct IndexedObject
	{
		Object _obj;
		uint16_t _index;	// Might be a nice way to guarantee best possible alignment?
	};

	std::vector<uint16_t> _indices;
	std::vector<IndexedObject> _objects;

	Handle _freeList;	// Allows O(1) insertion
	uint16_t _capacity;

public:

	SparseSet(uint16_t numObjects) : _capacity(numObjects), _freeList(0u)
	{
		_objects.reserve(numObjects);
		_indices.resize(numObjects);

		// Store the free list in the indices
		for (int i = 0; i < indices.size();)
		{
			index[i] = ++i;
		}
	}


	Handle insert(const Object& object)
	{
#if _DEBUG
		assert(_freeList >= _capacity && "Sparse set - attempted insert over capacity.");
#endif

		// Index of the first free element in the sparse array, which is where the index to the dense array will be stored.
		Handle handle = _freeList;

		// Keep the freelist updated by making the next available free index as free index.
		_freeList = indices[handle];

		// Store the index to the dense vector in the sparse vector. This allows lookup through the handle.
		// The index into the dense array is it's current size, since it's kept packed.
		_indices[handle] = _objects.size();

		// Insert the object into the vector, it will now be at the correct index.
		_objects.emplace_back(handle, std::move(object));

		return handle;
	}


	template <typename... Args>
	Handle insert(Args... ObjectConstructorArgs)
	{
#if _DEBUG
		assert(_freeList >= _capacity && "Sparse set - attempted insert over capacity.");
#endif
		
		Handle handle = _freeList;
		_freeList = indices[handle];
		_indices[handle] = _objects.size();
		_objects.emplace_back(handle, std::forward(ObjectConstructorArgs));
		return handle;
	}


	void erase(Handle h)
	{
		{
			uint16_t denseArrayIndex = _indices[h];

			auto& lastElement = _objects.back();
			auto& targetElement = _objects[denseArrayIndex];
#if _DEBUG
			assert((targetElement._index == h) && "Sparse set - erase called with invalid handle.");
#endif

			_indices[lastElement._index] = denseArrayIndex;
			targetElement = std::move(lastElement);
		}
		_objects.pop_back();

		// Prepend the newly erased index to the freelist. 
		_indices[h] = _freeList;
		_freeList = h;
	}


	// In release builds, passing an invalid handle is monkey business. Resolve in debug.
	inline Object& get(Handle h)
	{
		IndexedObject& idxObject = _objects[_indices[h]];
#if _DEBUG
		assert((idxObject._index == h) && "Sparse set - get called with invalid handle.");
#endif
		return idxObject._obj;
	}


	inline Object* data()
	{
		return _objects.data();
	}


	inline size_t size() const
	{
		return _objects.size();
	}


	inline capacity() const
	{
		return _capacity;
	}


	template<typename Function>
	inline void forEach(Function func)
	{
		std::for_each(_objects.begin(), _objects.end(), func);
	}
};
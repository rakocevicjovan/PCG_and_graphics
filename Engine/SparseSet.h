#pragma once
#include <vector>
#include <algorithm>
#include <assert.h>

typedef unsigned int UINT;

namespace
{
	template <typename HandleType>
	struct TypedHandleTemplate
	{
		uint16_t handle;
	};
}

// THIS IS NOT FINISHED AND _firstFree is absolutely not working right now.

// Preallocated vector that keeps contiguous elements using "swapping" (not really but close) and an index lookup vector
// Allows contiguous memory allocation with stable references to vector elements
// Using the handle after erase() is undefined.
template <typename Object>
class SparseSet
{
public:

	// Typed and untyped handle
	//typedef uint16_t Handle;
	typedef TypedHandleTemplate<Object> Handle;
	typedef uint16_t Index;

	struct IndexedObject
	{
		Object _obj;
		uint16_t _index;	// Might be a nice way to guarantee best possible alignment? Can use SOA as well, but this seems better.
	};

	SparseSet(const SparseSet&) = delete;
	SparseSet& operator=(const SparseSet&) = delete;

private:

	std::vector<uint16_t> _indices;
	std::vector<IndexedObject> _objects;

	uint16_t _freeList;	// Allows O(1) insertion
	uint16_t _capacity;

public:

	SparseSet(uint16_t numObjects = 100u) : _capacity(numObjects), _freeList(0u)
	{
		_objects.reserve(numObjects);
		_indices.resize(numObjects);

		// Store the free list in the indices
		for (int i = 0; i < _indices.size(); ++i)
		{
			_indices[i] = i + 1;
		}
	}


	Handle insert(Object&& object)
	{
#if _DEBUG
		assert(_freeList < _capacity && "Sparse set - attempted insert over capacity.");
#endif

		// Index of the first free element in the sparse array, which is where the index to the dense array will be stored.
		uint16_t handle = _freeList;

		// Keep the freelist updated by making the next available free index as free index.
		_freeList = _indices[handle];

		// Store the index to the dense vector in the sparse vector. This allows lookup through the handle.
		// The index into the dense array is it's current size, since it's kept packed.
		_indices[handle] = _objects.size();

		// Insert the object into the vector, it will now be at the correct index.
		_objects.push_back({ std::move(object), handle });

		return { handle };
	}


	template <typename... Args>
	Handle insert(Args&&... objectConstructorArgs)
	{
#if _DEBUG
		assert(_freeList < _capacity && "Sparse set - attempted insert over capacity.");
#endif
		
		auto handle = _freeList;
		_freeList = _indices[handle];
		_indices[handle] = _objects.size();
		_objects.push_back({ Object(std::forward<Args>(objectConstructorArgs)...), handle });
		return { handle };
	}


	void erase(Handle handle)
	{
		{
			uint16_t denseArrayIndex = _indices[handle];

			auto& lastElement = _objects.back();
			auto& targetElement = _objects[denseArrayIndex];
#if _DEBUG
			assert((targetElement._index == handle.handle) && "Sparse set - erase called with invalid handle.");
#endif

			_indices[lastElement._index] = denseArrayIndex;
			targetElement = std::move(lastElement);
		}
		_objects.pop_back();

		// Prepend the newly erased index to the freelist. 
		_indices[handle] = _freeList;
		_freeList = handle;
	}


	// In release builds, passing an invalid handle is monkey business. Resolve in debug.
	inline Object& get(Handle handle)
	{
		IndexedObject& idxObject = _objects[_indices[handle]];
#if _DEBUG
		assert((idxObject._index == handle.handle) && "Sparse set - get called with invalid handle.");
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


	inline size_t capacity() const
	{
		return _capacity;
	}


	template<typename Function>
	inline void forEach(Function func)
	{
		std::for_each(_objects.begin(), _objects.end(), func);
	}
};
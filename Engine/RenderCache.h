#pragma once
/*

// Might be used if there's an eed for more than a dumb wrapper.


#include "SparseSet.h"
#include "Renderable.h"

// Name is a bit too generic. In essence, only used for "Renderable", not for EVERY type of renderable entity
class RenderCache
{
public:

	explicit RenderCache(uint16_t _capacity) noexcept : _renderables(_capacity)
	{

	}

	// Probably not very useful on it's own. It will have to be able to take parameters as well, but writing overloads could be tiring...
	inline auto addRenderable(Renderable&& renderable)
	{
		return _renderables.insert(renderable);
	}

private:

	SparseSet<Renderable> _renderables;
};
*/
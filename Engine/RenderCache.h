#pragma once
#include "SparseSet.h"
#include "Renderable.h"

// Name is a bit too generic. In essence, only used for "Renderable", not for EVERY type of renderable entity
class RenderCache
{
public:

	// Probably not very useful on it's own. It will have to be able to take parameters as well, but writing overloads could be tiring...
	inline auto addRenderable(Renderable&& renderable)
	{
		return _renderables.insert(renderable);
	}

private:

	SparseSet<Renderable> _renderables;
};
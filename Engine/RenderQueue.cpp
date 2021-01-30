#include "RenderQueue.h"



RenderQueue::RenderQueue(size_t expectedMaximumSize)
{
	_renderables.reserve(expectedMaximumSize);
	_keys.reserve(expectedMaximumSize);
}



void RenderQueue::insert(const Renderable& renderable)
{
	_renderables.push_back(renderable);

	RenderQueueKey rqk;	// RenderQueueKey::createKey(0, renderable.mat);

	_keys.emplace_back(RenderQueueKey::createKey(1, 2, 3, 4, 5));	// Placeholder values, has no relation to anything
}



void RenderQueue::clear()
{
	_renderables.clear();
	_keys.clear();
}



void RenderQueue::sort()
{
	//determine how to sort by overloading < for renderable, but change this to work on keys instead
	//std::sort(_renderables.begin(), _renderables.end());

	std::sort(_keys.begin(), _keys.end());
}
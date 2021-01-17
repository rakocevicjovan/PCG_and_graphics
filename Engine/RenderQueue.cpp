#include "RenderQueue.h"



RenderQueue::RenderQueue(size_t maxElements) 
	: _maxElements(maxElements)
{}



void RenderQueue::add(Renderable& renderable)
{
	_renderables.push_back(renderable);
}



void RenderQueue::clear()
{
	_renderables.clear();
}



void RenderQueue::sort()
{
	//determine how to sort by overloading < for renderable, but change this to work on keys instead
	std::sort(_renderables.begin(), _renderables.end());
}
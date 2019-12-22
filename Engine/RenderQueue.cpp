#include "RenderQueue.h"



RenderQueue::RenderQueue(size_t maxOpaque, size_t maxTransparents) 
	: MAX_OPAQUES(maxOpaque), MAX_TRANSPARENTS(maxTransparents)
{}



void RenderQueue::add(Renderable& renderable)
{
	if (renderable.mat->opaque)
		opaques.push_back(renderable);	//stack allocator could work, or just reserving... not sure really
	else
		transparents.push_back(renderable);
}



void RenderQueue::clear()
{
	opaques.clear();
	transparents.clear();
}



void RenderQueue::sort()
{
	std::sort(opaques.begin(), opaques.end());	//determine how to sort by overloading < for renderable
	std::sort(transparents.begin(), transparents.end());
}
#pragma once
#include <vector>
#include "Renderable.h"


//can be a bunch of packed bits but do I really need it to be? this class can produce an int64_t eventually
class RenderableQueueKey
{
public:
	unsigned char renderTarget;		//max 256 render targets...
	uint16_t shaderSetId;			//max 65536 shader combinations
	uint16_t textureId;				//max 65536 texture combinations
	uint16_t depth;					//see how to shring float into uint16_t easily...
	unsigned char vertexFormat;		//max 256 vertex formats

	int64_t create64bitKey()
	{
		int64_t result =
			renderTarget << (63 - 8) |
			shaderSetId << (63 - 24) |
			textureId << (63 - 40) |
			depth << (63 - 56) |
			vertexFormat << (63 - 56);

		return result;
	}
};



class RenderQueue
{
	//std::vector<int64_t> qKeys;

	size_t MAX_OPAQUES = 500;
	size_t MAX_TRANSPARENTS = 100;

public:
	std::vector<Renderable> transparents;
	std::vector<Renderable> opaques;

	RenderQueue() = default;
	RenderQueue(size_t maxOpaque, size_t maxTransparent);

	void add(Renderable& renderable);
	void sort();
	void clear();
};
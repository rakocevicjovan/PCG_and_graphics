#pragma once
#include <vector>
#include "Renderable.h"

template <uint32_t RQKeyComponentSize>
inline constexpr uint32_t shiftOffset()
{
	static_assert(RQKeyComponentSize <= 64);
	return 63 - RQKeyComponentSize;
}

//can be a bunch of packed bits but do I really need it to be? this class can produce an int64_t eventually
class RenderableQueueKey
{
	static constexpr uint32_t renderTargetWidth = 8;

public:
	unsigned char renderTarget;		//max 256 render targets...
	uint16_t shaderSetId;			//max 65536 shader combinations
	uint16_t textureId;				//max 65536 texture combinations
	uint16_t depth;					//see how to shrink float into uint16_t easily...
	unsigned char vertexFormat;		//max 256 vertex formats


	typedef uint64_t KeyType;

	template <typename KeyFragment>
	inline void addKeyFragment(KeyType& key, uint16_t& bitOffset, KeyFragment keyFragment)
	{
		key |= static_cast<KeyType>(keyFragment) << bitOffset;
		bitOffset -= sizeof(keyFragment);
		assert((bitOffset > 0) && "Render queue key borked, it's too long.");
	}

	template <typename... KeyFragments>
	inline KeyType createKey(KeyFragments... keyFragments)
	{
		KeyType result{ 0 };
		uint16_t offset{ sizeof(KeyType) * 8 - 1 };

		((addKeyFragment(result, offset, keyFragments)), ...);
		return result;
	}

	inline uint64_t create64bitKey()
	{
		/* 
		This is possiblt a more flexible option, or introduce bit sizes in the fold expression method

			int64_t result =
				renderTarget << (63 - 8)	|
				shaderSetId << (63 - 24)	|
				textureId << (63 - 40)		|
				depth << (63 - 56)			|
				vertexFormat << (63 - 56);
		*/
		return createKey(renderTarget, shaderSetId, textureId, depth, vertexFormat);
	}
};



class RenderQueue
{
	size_t _maxElements;

public:
	std::vector<Renderable> _renderables;

	RenderQueue() = default;
	RenderQueue(size_t maxElements);

	void add(Renderable& renderable);
	void sort();
	void clear();
};
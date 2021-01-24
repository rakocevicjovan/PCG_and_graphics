#pragma once
#include <vector>
#include "Renderable.h"

class RenderQueue
{
private:

	// Use key fragment offsets/sizes as opposed to sizeof when it's clearer what to do.
	class RenderQueueKey
	{
	public:

		using KeyType = uint64_t;

	private:

		template <typename KeyFragment>
		inline static void addKeyFragment(KeyType& key, uint16_t& bitOffset, KeyFragment keyFragment)
		{
			key |= static_cast<KeyType>(keyFragment) << bitOffset;
			bitOffset -= sizeof(keyFragment);
			assert((bitOffset > 0) && "Render queue key borked, it's too long.");
		}

		template <typename... KeyFragments>
		inline static KeyType createKey_(KeyFragments... keyFragments)
		{
			KeyType result{ 0 };
			uint16_t offset{ sizeof(KeyType) * 8 - 1 };

			((addKeyFragment(result, offset, keyFragments)), ...);
			return result;
		}

public:

	inline static uint64_t createKey(UCHAR renderTarget, uint16_t shaderSetId, uint16_t textureId, uint16_t depth, UCHAR vertFormat)
	{
		return createKey_(renderTarget, shaderSetId, textureId, depth, vertFormat);
	}
	};

	std::vector<Renderable> _renderables;
	std::vector<RenderQueueKey::KeyType> _keys;

public:

	RenderQueue(size_t expectedMaximumSize = 512u);

	void insert(const Renderable& renderable);
	void sort();
	void clear();
};
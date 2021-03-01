#pragma once
#include "Math.h"
#include "PoolAllocator.h"

template <UINT MAX_DEPTH = 16u>
class PooledSceneGraph
{
public:

	// Linear data structure, keep sorted according to parent
	struct SGNode
	{
		SGNode* _parent;
		SMatrix _localTransform;
		SMatrix _globalTransform;
	};

private: 

	PoolAllocator<SGNode> _nodes;

public:

	PooledSceneGraph(UINT capacity = 1024u) : _nodes(capacity)
	{}

	// Parent can be null. Usually will be.
	SGNode* createNode(SMatrix transform, SGNode* parent)
	{
		SGNode* node = new (_nodes.allocate()) SGNode();
		
		node->_localTransform = transform;

		if (parent)
		{
			node->_parent = parent;
		}

		return node;
	}


	void removeNode(SGNode* node)
	{
		_nodes.deallocate(node);

		// How to erase without traversing everything... Might be better to control that from the outside
	}


	void update(SGNode* node)
	{
		static std::array<SMatrix, MAX_DEPTH> CHILD_STACK;
		static int ASCENT{ 0u };

		SGNode* parent = node->_parent;

		node->_globalTransform = node->_localTransform;

		// Update parent global transforms as well! Traverse back down (kinda sucks sure but this should not be deep)
		while (parent)
		{
			CHILD_STACK[ASCENT++] = parent->_localTransform;
			parent = parent->_parent;
		}

		for (; ASCENT > 0;)
		{
			node->_globalTransform = CHILD_STACK[--ASCENT] * node->_globalTransform;
		}
	}
};
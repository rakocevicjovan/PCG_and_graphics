#pragma once
#include "Hull.h"
#include <list>
#include <array>
#include <algorithm>

class OctNode
{
public:

	//either aabb or otherwise defined... not strictly needed if tree always splits in half, needed if irregular
	AABB bBox;
	
	//directly contained bounding volumes
	std::list<SphereHull*> hulls;
	
	//children nodes
	//std::array<OctNode*, 8> children;
	OctNode* children[8];

	//required? mabbeh...
	OctNode* parent;
};



class Octree
{
	int _maxDepth = 5;	//for preallocation
	int _threshold = 4;	//
	AABB _worldBounds;

public:
	OctNode* _rootNode;

	void init(const AABB& worldBounds, int maxDepth)
	{
		_worldBounds = worldBounds;
		_maxDepth = maxDepth;
	}



	void preallocateTree()
	{
		_rootNode = preallocateNode(_worldBounds.getPosition(), _worldBounds.getHalfSize(), _maxDepth, nullptr);
	}



	void prellocateRootOnly()
	{
		_rootNode = new OctNode();

		_rootNode->parent = nullptr;

		_rootNode->bBox = _worldBounds;	//kinda wasteful with copy construction...
	}



	AABB createBoxByIndex(int i, const AABB& parentBox)
	{
		SVec3 offset;
		SVec3 step = parentBox.getHalfSize() * 0.5f;
		offset.x = ((i & 1) ? step.x : -step.x);		//if odd, go right, if even, go left
		offset.y = ((i & 2) ? step.y : -step.y);		//pair down, pair up, pair down, pair up
		offset.z = ((i & 4) ? step.z : -step.z);		//four forward, four back

		return AABB(parentBox.getPosition() + offset, step);
	}



	int getIndexByPosition(const AABB& parentBox, const SVec3& pos)
	{
		SVec3 offset = parentBox.getPosition() - pos;
		return ( (offset.x > 0 ? 1 : 0) + (offset.y > 0 ? 2 : 0) + (offset.z > 0 ? 4 : 0) );	//kinda reverse of createBoxByIndex
	}



	OctNode* preallocateNode(SVec3 center, SVec3 halfSize, int stopDepth, OctNode* parent)
	{
		if (stopDepth < 0)
			return nullptr;

		OctNode* pNode = new OctNode();

		pNode->parent = parent;

		pNode->bBox = AABB(center, halfSize);	//kinda wasteful with copy construction...
		
		SVec3 offset;
		SVec3 step = halfSize * 0.5f;	//dimensions of aabb of parent, halved each step

		for (int i = 0; i < 8; ++i)
		{
			offset.x = ((i & 1) ? step.x : -step.x);
			offset.y = ((i & 2) ? step.y : -step.y);
			offset.z = ((i & 4) ? step.z : -step.z);
			pNode->children[i] = preallocateNode(center + offset, step, stopDepth - 1, parent);
		}
		return pNode;
	}



	void insertObject(OctNode* pNode, SphereHull* pSpHull)
	{
		int index = 0;
		bool straddle = 0;

		// Compute the octant number [0..7] the object sphere center is in
		// If straddling any of the dividing x, y, or z planes, exit directly
		for (int i = 0; i < 3; i++)
		{
			float delta = pSpHull->getPosition().at(i) - pNode->bBox.getPosition().at(i);	//distance - node middle to sphere middle
			if (abs(delta) <= pSpHull->r)	//pNode->bBox.getHalfSize().at(i)
			{
				straddle = 1;
				break;
			}
			if (delta > 0.0f)
				index |= (1 << i); // ZYX
		}


		if (!straddle)	// Fully contained in existing child node; insert in that subtree
		{
			//however, it could be empty! so... this, but it's not very good to do this without max depth checking
			if (pNode->children[index] == nullptr)
			{
				pNode->children[index] = new OctNode();
				pNode->children[index]->parent = pNode;
				pNode->children[index]->bBox = createBoxByIndex(index, pNode->bBox);
			}
			insertObject(pNode->children[index], pSpHull);
		}
		else
		{
			//from the book
			// Straddling, or no child node to descend into, so link object into linked list at this node
			//pObject->pNextObject = pNode->pObjList;
			//pNode->pObjList = pObject;
			
			//I did this another way because im using std::list<SphereHull*> instead of my object wrapper for hull
			pNode->hulls.push_back(pSpHull);
		}
	}



	bool removeObject(OctNode* pNode, SphereHull* pSpHull)
	{
		int index = getIndexByPosition(pNode->bBox, pSpHull->getPosition());

		bool straddle = 0;
		for (int i = 0; i < 3; i++)
		{
			float delta = pSpHull->getPosition().at(i) - pNode->bBox.getPosition().at(i);	//distance - node middle to sphere middle
			if (abs(delta) < pNode->bBox.getHalfSize().at(i) + pSpHull->r)
			{
				straddle = 1;
				break;
			}
			//if (delta > 0.0f) index |= (1 << i); // ZYX
		}

		//it's straddling, which means it's here and not in children (so far...)
		if (straddle)
		{
			pNode->hulls.erase(std::remove(pNode->hulls.begin(), pNode->hulls.end(), pSpHull));
		}

		if (pNode->children[index])
		{
			removeObject(pNode->children[index], pSpHull);
		}	
		else
		{
			return false;
		}
	}



	// Tests all objects that could possibly overlap due to cell ancestry and coexistence
	// in the same cell. Assumes objects exist in a single cell only, and fully inside it
	/*void TestAllCollisions(OctNode *pNode)
	{
		// Keep track of all ancestor object lists in a stack
		const int MAX_DEPTH = 40;
		static OctNode *ancestorStack[MAX_DEPTH];
		static int depth = 0; // ’Depth == 0’ is invariant over calls
		// Check collision between all objects on this level and all
		// ancestor objects. The current level is included as its own
		// ancestor so all necessary pairwise tests are done
		ancestorStack[depth++] = pNode;
		for (int n = 0; n < depth; n++) {
			Object *pA, *pB;
			for (pA = ancestorStack[n]->pObjList; pA; pA = pA->pNextObject) {
				for (pB = pNode->pObjList; pB; pB = pB->pNextObject) {
					// Avoid testing both A->B and B->A
					if (pA == pB) break;
					// Now perform the collision test between pA and pB in some manner
					TestCollision(pA, pB);
				}
			}
		}
		// Recursively visit all existing children
		for (int i = 0; i < 8; i++)
			if (pNode->pChild[i])
				TestAllCollisions(pNode->pChild[i]);
		// Remove current node from ancestor stack before returning
		depth--;
	}*/


	//for debugging purposes
	void getTreeAsAABBVector(std::vector<AABB>& AABBVector)
	{
		getNodeAABB(_rootNode, AABBVector);
	}



	void getNodeAABB(OctNode* pNode, std::vector<AABB>& AABBVector)
	{
		AABBVector.push_back(pNode->bBox);

		for (int i = 0; i < 8; ++i)
		{
			if (pNode->children[i])
				getNodeAABB(pNode->children[i], AABBVector);
		}
			
	}
};
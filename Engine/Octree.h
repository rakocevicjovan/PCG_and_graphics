#pragma once
#include "Hull.h"
#include <list>
#include <array>


//std::list here is a weakness tbh... need another way of storing and allocation of hulls, as well as nodes...
class OctNode
{
public:
	AABB bBox;						//not necessary if tree is split into even cubes, needed if irregular/loose...
	std::list<SphereHull*> hulls;	//directly contained bounding volumes
	OctNode* children[8];			//children nodes
};



class Octree
{
private:
	int _nodeCount = 1;	//root node is there by default
	int _maxDepth = 3;	//for preallocation and insertion/search max depths
	AABB _worldBounds;
	OctNode* _rootNode;

	//int _minHullsToSplit = 4;	//an alternative solution, currently using max subdivision depth to control splitsanity

	OctNode* preallocateNode(SVec3 center, SVec3 halfSize, int stopDepth, OctNode* parent);
	
	void updateNode(OctNode* node);
	void insertObjectIntoNode(OctNode* pNode, SphereHull* pSpHull, int depth = 0);
	bool removeObjectFromNode(OctNode* pNode, SphereHull* pSpHull);
	void trimNode(OctNode*& pNode);
	void deleteNode(OctNode*& pNode);
	bool isEmpty(OctNode* pNode);
	void getNodeAABB(OctNode* pNode, std::vector<AABB>& AABBVector);

	int getIndexByPosition(const AABB& parentBox, const SVec3& pos);
	AABB createBoxByIndex(int i, const AABB& parentBox);

public:
	~Octree();

	void init(const AABB& worldBounds, int maxDepth);
	void prellocateRootOnly();
	void preallocateTree();
	bool insertObject(SphereHull* pSpHull);
	bool removeObject(SphereHull* pSpHull);
	void lazyTrim();	//once per frame deallocate what's not required... would be faster with a pool allocator...
	void updateAll();

	void getTreeAsAABBVector(std::vector<AABB>& AABBVector);

	inline int getNodeCount() { return _nodeCount; }
};




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
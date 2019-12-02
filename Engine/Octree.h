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
	void testAllCollisions(OctNode *pNode);
	bool isEmpty(OctNode* pNode);
	void getNodeAABB(OctNode* pNode, std::vector<AABB>& AABBVector);

	int getIndexByPosition(const AABB& parentBox, const SVec3& pos);
	AABB createBoxByIndex(int i, const AABB& parentBox);

public:
	~Octree();

	void init(const AABB& worldBounds, int maxDepth);
	void prellocateRootOnly();
	void preallocateTree();
	void insertObject(SphereHull* pSpHull);
	bool removeObject(SphereHull* pSpHull);
	void collideAll();
	void lazyTrim();	//once per frame deallocate what's not required... would be faster with a pool allocator...
	void updateAll();

	void getTreeAsAABBVector(std::vector<AABB>& AABBVector);

	inline int getNodeCount() { return _nodeCount; }
};



// Tests all objects that could possibly overlap due to cell ancestry and coexistence in the same cell. 
//Assumes objects exist in a single cell only, and fully inside it, as this tree class is built
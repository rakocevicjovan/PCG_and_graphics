#pragma once
#include "Hull.h"
#include "ColFuncs.h"
#include "PoolAllocator.h"
#include <list>
#include <array>

// OCT_NODE_COUNT ( (pow(4, 8 + 1) - 1) / (4 - 1) )

//std::list here is a weakness tbh... need another way of storing and allocation of hulls, as well as nodes...
class OctNode
{
public:
	AABB _box;						// not necessary if tree is split into even cubes, needed if irregular/loose...
	std::list<SphereHull*> _hulls;	// directly contained bounding volumes, pointers are placement new-ed
	OctNode* _children[8];			// children nodes
	//bool _empty;					// not used yet, but would likely be a speed boost at almost no extra memory cost (aligns to 80)

	OctNode() : _children() {}
	OctNode(const AABB& aabb) : _box(aabb), _children() { /*sizeof(OctNode);*/ }

	void clear()
	{
		_hulls.clear();
		
		for (int i = 0; i < 8; ++i)
			if (_children[i])
				_children[i]->clear();
	}
};



class Octree
{
private:

	AABB _worldBounds;
	OctNode* _rootNode;
	PoolAllocator<OctNode> _octNodePool;

	// For preallocation and insertion/search max depths
	UINT _maxDepth = 3;
	//int _minHullsToSplit = 4;	// An alternative solution, currently using max subdivision depth to control splitsanity

	// Some statistics 
	UINT _nodeCount = 1;	// Root node is there by default
	UINT _hullCount = 0;

	inline UINT nodeCountFromDepth(UINT l)
	{
		return (static_cast<uint32_t>(std::pow(8, l + 1)) - 1u) / (8u - 1u);	// size is (N^(L+1)-1) / (N-1) where N = 8, L = depth (I'm using depth 8)
	}

	OctNode* preallocateNode(SVec3 center, SVec3 halfSize, int stopDepth, OctNode* parent);
	
	void updateNode(OctNode* node);
	void insertObjectIntoNode(OctNode* pNode, SphereHull* pSpHull, uint32_t depth = 0);
	bool removeObjectFromNode(OctNode* pNode, SphereHull* pSpHull);
	void trimNode(OctNode*& pNode);
	void deleteNode(OctNode*& pNode);
	void testAllCollisions(OctNode *pNode);
	bool isEmpty(OctNode* pNode) const;
	void getNodeAABB(OctNode* pNode, std::vector<AABB>& AABBVector);
	void rayCastNode(const OctNode* pNode, const SRay& lineSeg, const SRay& ray, std::list<SphereHull*>& spl) const;

	int getIndexByPosition(const AABB& parentBox, const SVec3& pos) const;
	AABB createBoxByIndex(UINT i, const AABB& parentBox) const ;

public:
	Octree(const AABB& worldBounds, UINT maxDepth);
	~Octree();
	

	void preallocateRootOnly();
	void preallocateTree();
	void insertObject(SphereHull* pSpHull);
	bool removeObject(SphereHull* pSpHull);
	void collideAll();
	void lazyTrim();	//once per frame, deallocate nodes that are not required... would be faster with a pool allocator...
	void updateAll();
	void rayCastTree(const SRay& ray, std::list<SphereHull*>& spl) const;
	void clear();

	void getTreeAsAABBVector(std::vector<AABB>& AABBVector);

	inline int getNodeCount() { return _nodeCount; }
	inline int getHullCount() { return _hullCount; }


//templates
public:
	template<typename NeighbourType>
	void findWithin(const SVec3& p, float r, std::vector<NeighbourType*>& neighbours) const
	{
		SphereHull sp(p, r);
		findInNode(_rootNode, sp, neighbours);
	}

private:
	template<typename NeighbourType>
	void findInNode(OctNode* pNode, const SphereHull& sp, std::vector<NeighbourType*>& neighbours) const
	{
		if (isEmpty(pNode))
			return;

		if(!Col::AABBSphereSimpleIntersection(pNode->_box, sp))
			return;

		for (SphereHull* curHull : pNode->_hulls)
			if ((curHull->getPosition() - sp.getPosition()).LengthSquared() < (sq(sp.getExtent() + curHull->getExtent())))
				neighbours.push_back(curHull->_collider->_parent);

		for (int i = 0; i < 8; i++)
			if (pNode->_children[i])
				findInNode(pNode->_children[i], sp, neighbours);
	}

};



// Tests all objects that could possibly overlap due to cell ancestry and coexistence in the same cell. 
//Assumes objects exist in a single cell only, and fully inside it, as this tree class is built
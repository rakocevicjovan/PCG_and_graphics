#include "pch.h"
#include "Octree.h"
#include "Collider.h"
#include "GameObject.h"


Octree::Octree(const AABB& worldBounds, UINT maxDepth) 
	: _maxDepth(maxDepth), _worldBounds(worldBounds), _octNodePool(nodeCountFromDepth(maxDepth))
{

}



Octree::~Octree()
{
	deleteNode(_rootNode);		//deletes everything
}



void Octree::preallocateTree()
{
	_rootNode = preallocateNode(_worldBounds.getPosition(), _worldBounds.getHalfSize(), _maxDepth, nullptr);
}



void Octree::preallocateRootOnly()
{
	_rootNode = new (_octNodePool.allocate()) OctNode(_worldBounds);
}



void Octree::deleteNode(OctNode*& pNode)
{
	//delete each existing child first, then self, to avoid "orphans" leaking memory
	for (int i = 0; i < 8; ++i)
	{
		if (pNode->_children[i])
			deleteNode(pNode->_children[i]);
	}

	--_nodeCount;
	_octNodePool.deallocate(pNode);
	pNode = nullptr;
}


// Once per frame deallocate what's not required... fairly fast with a pool allocator
void Octree::lazyTrim()
{
	// Avoids deleting root which is just nicer overall, less checks after
	for (int i = 0; i < 8; ++i)
	{
		if (_rootNode->_children[i])
			trimNode(_rootNode->_children[i]);
	}
}



void Octree::trimNode(OctNode*& pNode)
{
	for (int i = 0; i < 8; ++i)
	{
		if (pNode->_children[i])
			trimNode(pNode->_children[i]);
	}

	if (isEmpty(pNode))
		deleteNode(pNode);
}



bool Octree::isEmpty(OctNode* pNode) const
{
	// Contains collider hulls, no need to check the children
	if (!pNode->_hulls.empty())
		return false;

	// Contains no hulls directly, check children
	for (int i = 0; i < 8; ++i)
		if (pNode->_children[i] != nullptr)
			return false;

	// Node is considered empty if and only if all children are nullptr and hulls list is empty
	return true;
}



AABB Octree::createBoxByIndex(UINT i, const AABB& parentBox) const
{
	SVec3 offset;
	SVec3 step = parentBox.getHalfSize() * 0.5f;
	offset.x = ((i & 1u) ? step.x : -step.x);		// If odd go right, if even go left
	offset.y = ((i & 2u) ? step.y : -step.y);		// Pair down, pair up, pair down, pair up
	offset.z = ((i & 4u) ? step.z : -step.z);		// Four back, four forward

	return AABB(parentBox.getPosition() + offset, step);
}



int Octree::getIndexByPosition(const AABB& parentBox, const SVec3& pos) const
{
	SVec3 offToParent = parentBox.getPosition() - pos;
	// Right gets odd, up gets 2, foward gets 4, bitmask goes from 000 nbl to 111 ftr
	return (offToParent.x > 0) * 1. + (offToParent.y > 0) * 2 + (offToParent.z > 0) * 4;
}



OctNode* Octree::preallocateNode(SVec3 center, SVec3 size, int stopDepth, OctNode* parent)
{
	if (stopDepth < 0)
		return nullptr;

	OctNode* pNode = new (_octNodePool.allocate()) OctNode(AABB(center, size));

	++_nodeCount;

	SVec3 offset;
	SVec3 halfSize = size * 0.5f;	//dimensions of aabb of parent, halved each step

	for (int i = 0; i < 8; ++i)
	{
		offset.x = ((i & 1) ? halfSize.x : -halfSize.x);
		offset.y = ((i & 2) ? halfSize.y : -halfSize.y);
		offset.z = ((i & 4) ? halfSize.z : -halfSize.z);
		pNode->_children[i] = preallocateNode(center + offset, halfSize, stopDepth - 1, parent);
	}
	return pNode;
}



void Octree::insertObject(SphereHull* pSpHull)
{
	++_hullCount;
	insertObjectIntoNode(_rootNode, pSpHull);
}



void Octree::insertObjectIntoNode(OctNode* pNode, SphereHull* pSpHull, int depth)
{
	int index = 0;
	bool straddle = 0;

	// Compute the octant number [0..7] the object sphere center is in
	// If straddling any of the dividing x, y, or z planes, exit directly
	for (int i = 0; i < 3; i++)
	{
		float delta = pSpHull->getPosition().at(i) - pNode->_box.getPosition().at(i);	//distance - node middle to sphere middle
		if (abs(delta) <= pSpHull->getExtent())	//pNode->bBox.getHalfSize().at(i)
		{
			straddle = 1;
			break;
		}
		if (delta > 0.0f)
			index |= (1 << i); // ZYX
	}


	if (!straddle && (depth < _maxDepth))	// Fully contained in existing child node; insert in that subtree
	{
		//however, it could be empty! so... this, but it's not very good to do this without max depth checking
		if (pNode->_children[index] == nullptr)
		{
			pNode->_children[index] = new (_octNodePool.allocate()) OctNode(createBoxByIndex(index, pNode->_box));
			++_nodeCount;
		}
		insertObjectIntoNode(pNode->_children[index], pSpHull, ++depth);
		return;
	}

	// From the book
	// Straddling, or no child node to descend into, so link object into linked list at this node
	// pObject->pNextObject = pNode->pObjList;
	// pNode->pObjList = pObject;

	// I did this another way because im using std::list<SphereHull*> instead of my object wrapper for hull
	pNode->_hulls.push_back(pSpHull);
}



bool Octree::removeObject(SphereHull* pSpHull)
{
	if (removeObjectFromNode(_rootNode, pSpHull))
	{
		--_hullCount;
		return true;
	}
	return false;
}



void Octree::collideAll()
{
	testAllCollisions(_rootNode);
}



bool Octree::removeObjectFromNode(OctNode* pNode, SphereHull* pSpHull)
{
	int index = getIndexByPosition(pNode->_box, pSpHull->getPosition());

	bool straddle = 0;
	for (int i = 0; i < 3; ++i)
	{
		float delta = pSpHull->getPosition().at(i) - pNode->_box.getPosition().at(i);	//distance - node middle to sphere middle
		if (abs(delta) < pNode->_box.getHalfSize().at(i) + pSpHull->getExtent())
		{
			straddle = 1;
			break;
		}
		//if (delta > 0.0f) index |= (1 << i); // ZYX
	}

	//it's straddling, which means it's here and not in children (so far...)
	if (straddle)
	{
		pNode->_hulls.remove(pSpHull);//(std::remove(pNode->hulls.begin(), pNode->hulls.end(), pSpHull));
		return true;
	}

	if (pNode->_children[index])
		removeObjectFromNode(pNode->_children[index], pSpHull);
	else
		return false;
}



void Octree::updateAll()
{
	updateNode(_rootNode);
}



//Bugged! It works, but they can be possibly reinserted twice, and it's not making use of locality! Very important @TODO!
void Octree::updateNode(OctNode* node)
{
	// @TODO Change this to simply be reinserted from outside the tree...
	std::list<SphereHull*> wat = std::move(node->_hulls);

	std::list<SphereHull*>::iterator iter;
	for (iter = wat.begin(); iter != wat.end(); ++iter)
	{
		insertObjectIntoNode(_rootNode, (*iter), 0);
	}

	for (auto& child : node->_children)
		if (child != nullptr)
			updateNode(child);
}



void Octree::testAllCollisions(OctNode *pNode)
{
	// Keep track of all ancestor object lists in a stack
	const int MAX_DEPTH = 10;
	static OctNode* ancestorStack[MAX_DEPTH];
	static int depth = 0;

	// Check collision between all objects on this level and all ancestor objects. 

	// Empty nodes have no hulls OR children so we can just cut it off right there
	if (isEmpty(pNode))
		return;

	// Include current node into the ancestry stack.
	ancestorStack[depth++] = pNode;

	for (int n = 0; n < depth; n++)	// Iterate the ancestor stack
	{
		for (SphereHull* spA : ancestorStack[n]->_hulls)	// Check all hulls in ancestors...
		{
			// Exit early if either is not collidable (@TODO INDIRECTION HERE CAN BE COSTLIER THAN CHECK!)
			if (!(spA->_collider->collidable))	
				continue;

			for (SphereHull* spB : pNode->_hulls)			// ...against hulls in the current node
			{
				// Prevent self-collision. Book says break but that is incorrect (confirmed by errata)
				if (spA == spB)	
					continue;

				if (!(spB->_collider->collidable))
					continue;

				//What to do with the hit result now? @TODO Separate response from detection!
				//collider seems to be a good candidate to hold response logic and definition
				HitResult hr = Col::SphereSphereIntersection(*spA, *spB);
				
				if (hr.hit)
				{
					// Breaks apart if actors relocate... consider between indices and allocators
					if (spA->_collider->dynamic)
					{
						spA->setPosition(spA->getPosition() + hr.resolutionVector * .5);
						Math::SetTranslation(spA->_collider->_parent->_transform, spA->getPosition());
					}
					if (spB->_collider->dynamic)
					{
						spB->setPosition(spB->getPosition() - hr.resolutionVector * .5);
						Math::SetTranslation(spB->_collider->_parent->_transform, spB->getPosition());
					}
				}
			}
		}
	}

	// Recursively visit all existing children
	for (int i = 0; i < 8; i++)
	{
		if (pNode->_children[i])
			testAllCollisions(pNode->_children[i]);
	}

	// Remove current node from ancestor stack before returning
	--depth;
}



//for debugging purposes
void Octree::getTreeAsAABBVector(std::vector<AABB>& AABBVector)
{
	getNodeAABB(_rootNode, AABBVector);
}



void Octree::getNodeAABB(OctNode* pNode, std::vector<AABB>& AABBVector)
{
	if (!pNode->_hulls.empty())
		AABBVector.push_back(pNode->_box);

	for (int i = 0; i < 8; ++i)
		if (pNode->_children[i])
			getNodeAABB(pNode->_children[i], AABBVector);
}


// Needs full length ray! This fn converts the given ray to a line segment internally for now
void Octree::rayCastTree(const SRay& ray, std::list<SphereHull*>& spl) const
{
	rayCastNode(_rootNode, SRay(ray.position, ray.position + ray.direction), ray, spl);
}



void Octree::rayCastNode(const OctNode* pNode, const SRay& lineSeg, const SRay& ray, std::list<SphereHull*>& spl) const
{
	if (!Col::LSegmentAABBSimpleIntersection(lineSeg, pNode->_box))
		return;

	for (SphereHull* sp : pNode->_hulls)
	{
		if (Col::RaySphereIntersection(ray, *sp))
			spl.push_back(sp);
	}

	for (int i = 0; i < 8; ++i)
		if (pNode->_children[i])
			rayCastNode(pNode->_children[i], lineSeg, ray, spl);
}



void Octree::clear()
{
	_hullCount = 0u;
	_rootNode->clear();
}
#include "QuadTree.h"



void QTNode::setChild(uint32_t index, QTNode * pNode)
{
}

bool QTNode::addObject(QTObject* pObj)
{
	return true;
}

void QTNode::remObject(QTObject * pObj)
{
}



bool QuadTree::create(float fSize)
{
	_invRadius = static_cast<float>(255.f) / fSize;

	reset();

	_nodePool = new uint8_t[QT_NODE_COUNT * sizeof(QTNode)];

	//allocation failed - exit
	if (!_nodePool)
		return false;

	QTNode* rNode = root();

	//placement new all nodes in the pool
	for (uint32_t i = QT_NODE_COUNT; i--; )
		new(&rNode[i]) QTNode();


	//set up the loop - root has no ptr, so last node is null
	QTNode* thisNode = rNode;
	QTNode* lastNode = nullptr;

	//we use radius to denote half size of nodes so we need to divide
	float fullSize = fSize;
	float halfSize = fSize * 0.5f;
	_radius = halfSize;

	//centers the quadtree by moving all positions bottom left by half size
	SVec2 vOffset(-halfSize, -halfSize);

	//iterate through 8 levels
	for (uint32_t i = 0u; i < 8u; ++i)
	{
		_levels[i] = thisNode;

		UINT rowSize = (1UL << i);	//1, 2, 4, 8... power of two to get the node count of each row

		// For each node at this level (across both for loops within)
		for (uint32_t y = rowSize; y--; )
		{
			//starting address of the row - first level: 1 * 1, second level: 2 * 2
			QTNode* row = &thisNode[y * rowSize];

			//first level's position is is 1 / 1 * fullSize + halfSize = fullSize + halfSize
			float yPos = float(y) / float(rowSize) * fullSize + halfSize;

			//x goes from row size to 0, same as y
			for (uint32_t x = rowSize; x--; )	//1, 2, 4, 8 again, descending down the tree
			{
				row[x].setCenterAndRadius(SVec2(float(x) / float(rowSize) * fullSize + halfSize, yPos) + vOffset, halfSize);

				if (lastNode)
				{
					// There is a parent.  Get its index.
					uint32_t ui32ParentX = x >> 1UL;
					uint32_t ui32ParentY = y >> 1UL;
					uint32_t ui32ParentIndex = (ui32ParentY * (1ul << (i - 1ul))) + ui32ParentX;
					uint32_t ui32X = x & 1UL;
					uint32_t ui32Y = y & 1UL;
					lastNode[ui32ParentIndex].setChild((ui32Y << 1ul) + ui32X, &row[x]);
				}
			}
		}

		lastNode = thisNode;
		thisNode += rowSize * rowSize;
		halfSize *= 0.5f;
	}

	return true;
}



void QuadTree::reset()
{
	if (_nodePool)
	{
		QTNode* nodes = root();

		//Relying on undeflow making it bigger than the node count? meh... 
		for (uint32_t i = QT_NODE_COUNT; i--;)	
		{
			nodes[i].~QTNode();
		}
		
		delete[] _nodePool;
		_nodePool = nullptr;
	}
}


//this makes sure anything outside the octree still ends up in the root to avoid awkward cases later
bool QuadTree::insert(QTObject* pObject)
{
	/* uncomment when ready to finish
	const AABB2D& box = pObject->getQuadTreeRect();

	//consider storing 2 SVec2s to avoid eye bleed... structs are aligned anyways so not saving any memory as is
	if (box._min.x < -_radius || box._max.x > _radius || box._min.y < -_radius || box._max.y > _radius)
		return root()->addObject(pObject);
	*/
	return false;
}



//protected
QTNode* QuadTree::root()
{
	return reinterpret_cast<QTNode*>(&_nodePool[0]);
}



//public read access, can't modify it
const QTNode* QuadTree::cRoot() const
{
	return reinterpret_cast<const QTNode*>(&_nodePool[0]);
}
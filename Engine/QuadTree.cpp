#include "pch.h"
#include "QuadTree.h"
#include "Bits.h"



bool QuadTree::init(float fSize)
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
			//starting address of the row - first level: at 0 then at 1
			QTNode* row = &thisNode[y * rowSize];

			//first level's position is is 1 / 1 * fullSize + halfSize = fullSize + halfSize
			float yPos = float(y) / float(rowSize) * fullSize + halfSize;

			//x goes from row size to 0, same as y
			for (uint32_t x = rowSize; x--; )	//1, 2, 4, 8 again, descending down the tree
			{
				row[x].setCenterAndRadius(SVec2(float(x) / float(rowSize) * fullSize + halfSize, yPos) + vOffset, halfSize);

				//first level (root node, pos 0) does not enter this, all the others do
				if (lastNode)
				{
					// There is a parent.  Get its index. Divide x and y by two to address previous level.
					uint32_t ui32ParentX = x >> 1UL;
					uint32_t ui32ParentY = y >> 1UL;

					//left shift 1 to (i-1) to get the row of the parent (for i=0: 0 * (1 << 0) + 0 = 0
					uint32_t ui32ParentIndex = (ui32ParentY * (1ul << (i - 1ul))) + ui32ParentX;

					//not sure exactly how this works... it maps 0 to 0 and anything else to 1
					//we get 4 children at 00, 01, 10, 11 true but how
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


//untested, should see if I ported it right from L. Spiro's tutorial
bool QuadTree::insert(QTObject* pObject)
{
	const AABB2D& box = pObject->getQuadTreeRect();

	//this makes sure anything outside the octree still ends up in the root to avoid awkward cases later
	if (box._min.x < -_radius || box._max.x > _radius || box._min.y < -_radius || box._max.y > _radius)
		return root()->addObject(pObject);

	// The quad-tree exists in the range from -m_fRadius to m_fRadius.
	//	Convert to the range 0 to (m_fRadius * 2).
	AABB2D a2Shifted;
	a2Shifted._min = box._min + SVec2(_radius, _radius);
	a2Shifted._max = box._max + SVec2(_radius, _radius);

	// Now to the range from [0..255].
	a2Shifted._min.x *= _invRadius;
	a2Shifted._min.y *= _invRadius;
	a2Shifted._max.x *= _invRadius;
	a2Shifted._max.y *= _invRadius;

	// Convert to integers and clamp.
	uint32_t ui32MinX = static_cast<uint32_t>(Math::clamp(floor(a2Shifted._min.x), 0, static_cast<float>(255.f)));
	uint32_t ui32MaxX = static_cast<uint32_t>(Math::clamp(ceil(a2Shifted._max.x), 0, static_cast<float>(255.f)));

	uint32_t ui32MinY = static_cast<uint32_t>(Math::clamp(floor(a2Shifted._min.y), 0, static_cast<float>(255.f)));
	uint32_t ui32MaxY = static_cast<uint32_t>(Math::clamp(ceil(a2Shifted._max.y), 0, static_cast<float>(255.f)));

	// Get the level at which the object will be inserted.
	//XOR min and max values, kinda mapping the bits to exists/doesn't along the axis (
	uint32_t ui32X = ui32MinX ^ ui32MaxX;	
	if (!ui32X)
		ui32X = 7UL;	// 100% flat objects go to the highest (smallest) level.
	else
		ui32X = 7UL - Bits::msbDeBruijn32(ui32X);

	uint32_t ui32Y = ui32MinY ^ ui32MaxY;
	if (!ui32Y)
		ui32Y = 7UL;	// 100% flat objects go to the highest (smallest) level.
	else
		ui32Y = 7UL - Bits::msbDeBruijn32(ui32Y);

	uint32_t ui32Level = min(ui32X, ui32Y);

	// Now we know which level in the tree it is.
	// Find out which node on that level owns it.
	ui32X = ui32MinX >> (8UL - ui32Level);
	ui32Y = ui32MinY >> (8UL - ui32Level);

#ifdef _QTDEBUG
	QTNode* pNode = &_levels[ui32Level][ui32Y*(1UL << ui32Level) + ui32X];
	assert(pNode->getCenter().x - pNode->getRadius() <= box.m_vMin.x);
	assert(pNode->getCenter().x + pNode->getRadius() >= box.m_vMax.x);
	assert(pNode->getCenter().y - pNode->getRadius() <= box.m_vMin.y);
	assert(pNode->getCenter().y + pNode->getRadius() >= box.m_vMax.y);
#endif	// #ifdef _DEBUG
	return _levels[ui32Level][ui32Y*(1UL << ui32Level) + ui32X].addObject(pObject);

	return true;
}



//protected
QTNode* QuadTree::root()
{
	return reinterpret_cast<QTNode*>(&_nodePool[0]);
}



//public read access, can't modify it outside of class, added c to indicate const and avoid confusion
const QTNode* QuadTree::rootC() const
{
	return reinterpret_cast<const QTNode*>(&_nodePool[0]);
}



void QTNode::setChild(uint32_t index, QTNode * pNode)
{
	_children[index] = pNode;
}


bool QTNode::addObject(QTObject* pObj)
{
	_objects.push_back(pObj);
	return false;
}


void QTNode::remObject(QTObject* pObj)
{
	_objects.remove(pObj);
}
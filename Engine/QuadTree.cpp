#include "QuadTree.h"



bool QTNode::addObject(QTObject* pObj)
{

}



bool QuadTree::init(float size)
{

}



void QuadTree::reset()
{
	if (_nodes)
	{
		QTNode* nodes = root();

		//Relying on undeflow making it bigger than the node count? meh... 
		for (uint32_t i = QT_NODE_COUNT; i--;)	
		{
			nodes[i].~QTNode();
		}
		
		delete[] _nodes;
		_nodes = nullptr;
	}
}


//this makes sure anything outside the octree still ends up in the root to avoid awkward cases later
bool QuadTree::insert(QTObject* pObject)
{
	const AABB2D& box = pObject->getQuadTreeRect();

	//consider storing 2 SVec2s to avoid eye bleed... structs are aligned anyways so not saving any memory as is
	if (box._c.x - box._r < -_radius || box._c.x + box._r > +_radius || box._c.y - box._r < -_radius || box._c.y + box._r > _radius)
		return root()->addObject(pObject);

	
}



//protected
QTNode* QuadTree::root()
{
	return _levels[0];
}



//public read access, can't modify it
const QTNode* QuadTree::cRoot() const
{
	return _levels[0];
}
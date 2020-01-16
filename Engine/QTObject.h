#pragma once
#include "AABB2D.h"

class QTNode;

//parent to anything we are throwing into the quadtree, each object must implement getQuadTreeRect()
//although this is not enforced by pure virtual for now, comment should be enough
class QTObject
{
protected:

	QTNode* _qtNode;

public:

	QTObject()
	{
	}


	inline const QTNode* getQuadTreeNode() const
	{
		return _qtNode;
	}


	inline void setQuadTreeNode(QTNode* pNode)
	{
		_qtNode = pNode;
	}


	virtual const AABB2D& getQuadTreeRect() const;
};

#pragma once
#include "Math.h"
#include <list>
#include <vector>



//parent to anything we are throwing into the quadtree
class QTObject
{
protected:

	QTNode* _qtNode;

public:
	QTObject() {}

	void setQuadTreeNode();

	virtual const AABB2D& getQuadTreeRect() const;
	inline const QTNode* getQuadTreeNode() const;

};



class AABB2D
{
public:
	SVec2 _c;
	float _r;
};



class QTNode
{
	friend class QuadTree;

protected:
	AABB2D _rect;
	std::list<QTObject*> _objects;
	QTNode* _children[4];

public:

	inline void setCenterAndRadius(const SVec2& ctr, float radius)
	{
		_rect._c = ctr;
		_rect._r = radius;
	}

	inline const SVec2& getCenter() { return _rect._c; }
	inline float getRadius() { return _rect._r; }

	void setChild(uint32_t index, QTNode* pNode);
	bool addObject(QTObject* pObj);
	void remObject(QTObject* pObj);
};



class QuadTree
{
protected:
	//size is (N^(L+1)-1) / (N-1) where N = 4, L = depth (I'm using depth 8)
	#define QT_NODE_COUNT ( (pow(4, 8 + 1) - 1) / (4 - 1) )


	//std::vector<QTNode> _nodes;
	//QTNode* _nodes;
	uint32_t* _nodes;

	QTNode* _levels[8];
	float _radius;
	float _invRadius;
	
	inline QTNode* root();

public:

	bool init(float size);
	void reset();
	bool insert(QTObject* pObject);

	const QTNode* cRoot() const;


	QuadTree() : _nodes(nullptr)
	{}

	~QuadTree()
	{
		reset();
	}
};







//static constexpr uint32_t NODE_COUNT = positiveOnlyPow(4, 8 + 1) - 1 / (4 - 1);
//the "modern" way relies on recursion (C++11 at least) and reads like crap so... good ole #define to the rescue

/*
constexpr uint32_t positiveOnlyPow(int b, int e) noexcept
{
	uint32_t result = 1;

	for (UINT i = 0; i < e; ++e)
		result *= b;

	return result;


constexpr uint32_t positiveOnlyPow(int b, int e) noexcept
{
	return (e == 0 ? 1 : b * positiveOnlyPow(b, e - 1));
}
}*/
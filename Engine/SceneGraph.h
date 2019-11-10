#pragma once
#include "GameObject.h"



template <typename Content>
class Node
{
public:
	Content content;
	Node* parent;
	Node* children[];

	Node(Content c)
	{
		content = c;
	}
};



class SceneGraph
{
private:
	Node<GameObject> _rootNode;

public:

	template <typename GraphItem>
	void insert(GraphItem graphItem)
	{
		Node newNode(graphItem);
		newNode.parent = _rootNode;
	}

	void sortByViewDepth();
	void clear();
};
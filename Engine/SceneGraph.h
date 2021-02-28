#pragma once
#include "GameObject.h"
#include <vector>

class NodeBase
{
public:
	NodeBase* _parent;
	std::vector<NodeBase*> _children;
};



template<class Content>
class Node : public NodeBase
{
public:
	Content _content;

	Node(Content c): _content(c) {}
};



class SceneGraph
{
private:
	NodeBase _rootNode;

public:

	SceneGraph()
	{
	}

	

	template <typename GraphItem>
	NodeBase* insert(GraphItem graphItem)
	{
		//create the node on the heap
		Node<GraphItem>* newNode = new Node<GraphItem>(graphItem);	//don't do this, make node OR object own this...

		//attach node to the parent, and parent to the node
		newNode->parent = &_rootNode;
		_rootNode.children.push_back(newNode);

		//return the created node pointer so that the creating item can be aware of it's node
		return newNode;
	}



	void erase(NodeBase* pNodeBase)
	{
		auto& siblings = pNodeBase->_parent->_children;
		siblings.erase(remove(siblings.begin(), siblings.end(), pNodeBase), siblings.end());
		
		for (auto c : pNodeBase->_children)
			erase(c);

		delete pNodeBase;
	}



	void clear()
	{
		// tbd
	}
};
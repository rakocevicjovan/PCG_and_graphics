#pragma once
#include "GameObject.h"

namespace SG
{

template <typename Content>
struct Node
{
	Content node;
	Node* parent;
	Node* children[];
};



class SceneGraph
{
	Node<GameObject> _rootNode;
};

}
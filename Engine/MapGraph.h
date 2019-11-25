#pragma once
#include "Math.h"
#include <vector>


//should be a convex shape... how to do this though, is a big ???
struct MGNode
{
	std::vector<SVec3> MGEdge;
};



struct MGEdge
{
	//two fields that share this border
	MGNode* left;
	MGNode* right;

};



class MapGraph
{


};